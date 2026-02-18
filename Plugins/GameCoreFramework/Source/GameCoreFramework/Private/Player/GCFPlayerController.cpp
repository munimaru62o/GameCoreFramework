// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#include "Player/GCFPlayerController.h"

#include "GCFShared.h"
#include "Player/GCFPlayerState.h"
#include "Player/GCFLocalPlayer.h"
#include "Actor/Pawn/GCFPawn.h"
#include "Actor/Data/GCFPawnData.h"
#include "Components/PrimitiveComponent.h"
//#include "GCFCheatManager.h"
//#include "Camera/GCFPlayerCameraManager.h"
#include "UI/GCFHUD.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "AbilitySystemGlobals.h"
#include "System/GCFGameState.h"
#include "Gameframework/GameUserSettings.h"
#include "CommonInputTypeEnum.h"
#include "CommonInputSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Player/GCFControllerPossessionComponent.h"
#include "Input/GCFInputBindingManagerComponent.h"
#include "Input/GCFPlayerInputBridgeComponent.h"
#include "Input/GCFInputContextComponent.h"
#include "Input/GCFAbilityInputRouterComponent.h"
#include "System/Lifecycle/GCFPossessionContextComponent.h"
#include "AbilitySystem/GCFAbilitySystemFunctionLibrary.h"
#include "Camera/GCFCameraControlComponent.h"
#include "Movement/GCFMovementControlComponent.h"
#include "Messages/GCFGameplayMessages.h"

#include "MoverComponent.h"
#include "Backends/MoverNetworkPredictionLiaison.h"
#include "Components/SphereComponent.h"
#include "Movement/Mover/GCFDummyMovementMode.h"

#include "System/GCFDeveloperSettings.h"
#include "GameMapsSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFPlayerController)

namespace GCF
{
	namespace Input
	{
		static int32 ShouldAlwaysPlayForceFeedback = 0;
		static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("GCFPC.ShouldAlwaysPlayForceFeedback"),
			ShouldAlwaysPlayForceFeedback,
			TEXT("Should force feedback effects be played, even if the last input device was not a gamepad?"));
	}
}

AGCFPlayerController::AGCFPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//PlayerCameraManagerClass = AGCFPlayerCameraManager::StaticClass();

	bAutoManageActiveCameraTarget = false;

	InputBindingManagerComponent = CreateDefaultSubobject<UGCFInputBindingManagerComponent>(TEXT("InputBindingManagerComponent"));
	PlayerInputBridgeComponent = CreateDefaultSubobject<UGCFPlayerInputBridgeComponent>(TEXT("PlayerInputBridgeComponent"));
	InputContextComponent = CreateDefaultSubobject<UGCFInputContextComponent>(TEXT("InputContextComponent"));
	PossessionContextComponent = CreateDefaultSubobject<UGCFPossessionContextComponent>(TEXT("PossessionContextComponent"));
	ControllerPossessionComponent = CreateDefaultSubobject<UGCFControllerPossessionComponent>(TEXT("ControllerPossessionComponent"));
	AbilityInputRouterComponent = CreateDefaultSubobject<UGCFAbilityInputRouterComponent>(TEXT("AbilityInputRouterComponent"));
	
	// -----------------------------------------------------------------------------------------
	// [Architectural Note: NPP Clock Sync Workaround]
	// 
	// WHY THIS IS NECESSARY:
	// In Unreal Engine 5's Network Prediction Plugin (NPP), the simulation clock is isolated 
	// per ModelDef. If a client possesses a pawn that does NOT use Mover (e.g., standard CMC), 
	// the Mover-specific NPP clock goes to sleep. Consequently, if the server spawns a 
	// Simulated Proxy using Mover (like a drone or vehicle), the client will discard its 
	// packets as "too far in the future," causing the proxy to freeze (Extrapolation Starvation) 
	// or infinitely overshoot.
	// 
	// HOW IT WORKS:
	// By keeping a collision-less, dummy MoverComponent on the PlayerController (which is always 
	// an Autonomous Proxy), we force the Mover's NPP clock to constantly tick and synchronize 
	// with the server, regardless of which pawn the player is actually possessing.
	// 
	// STATUS:
	// This was initially a temporary measure but is now formally adopted as a required adapter 
	// for our hybrid movement architecture (CMC + Mover) until Epic Games provides a native 
	// solution for global/decoupled NPP clock synchronization.
	// -----------------------------------------------------------------------------------------
	USphereComponent* DummyRoot = CreateDefaultSubobject<USphereComponent>(TEXT("DummyRoot"));
	DummyRoot->InitSphereRadius(1.0f);
	DummyRoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = DummyRoot;

	MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("DummyMoverComponent"));
	MoverLiaisonComponent = CreateDefaultSubobject<UMoverNetworkPredictionLiaisonComponent>(TEXT("DummyMoverLiaisonComponent"));

#if USING_CHEAT_MANAGER
	CheatClass = UGCFCheatManager::StaticClass();
#endif // #if USING_CHEAT_MANAGER
}


void AGCFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	OnInputComponentReady.Broadcast(InputComponent);
}


void AGCFPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (UGCFLocalPlayer* LocalPlayer = Cast<UGCFLocalPlayer>(Player)) {
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, InPawn);
	}
}

void AGCFPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// -----------------------------------------------------------------------------------------
	// [NPP Clock Sync Workaround: Silence Warning & Ensure Simulation]
	// Mover requires at least one MovementMode to run its simulation loop without throwing warnings.
	// We use NewObject here (instead of CreateDefaultSubobject in the constructor) to safely 
	// inject a dummy UObject at runtime without corrupting the Class Default Object (CDO).
	// -----------------------------------------------------------------------------------------
	if (MoverComponent) {
		UGCFDummyMovementMode* DummyMode = NewObject<UGCFDummyMovementMode>(MoverComponent, TEXT("DummyMode"));
		MoverComponent->MovementModes.Add(TEXT("Dummy"), DummyMode);
		MoverComponent->StartingMovementMode = TEXT("Dummy");
	}
}

void AGCFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld()) {
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
		MessageHandle = MakeUnique<FGCFMessageSubscription>(
			World,
			MessageSubsystem.RegisterListener<FGCFPolicyChangedCursorMessage>(
			GCFGameplayTags::Message_PolicyChange_Cursor,
			this,
			&ThisClass::OnCameraModeMessageReceived));
	}
}

void AGCFPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MessageHandle.Reset();
	Super::EndPlay(EndPlayReason);
}

void AGCFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable replicating the PC target view as it doesn't work well for replays or client-side spectating.
	// The engine TargetViewRotation is only set in APlayerController::TickActor if the server knows ahead of time that 
	// a specific pawn is being spectated and it only replicates down for COND_OwnerOnly.
	// In client-saved replays, COND_OwnerOnly is never true and the target pawn is not always known at the time of recording.
	// To support client-saved replays, the replication of this was moved to ReplicatedViewRotation and updated in PlayerTick.
	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

void AGCFPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (UGCFLocalPlayer* LocalPlayer = Cast<UGCFLocalPlayer>(Player)) {
		LocalPlayer->OnPlayerControllerSet.Broadcast(LocalPlayer, this);

		if (PlayerState) {
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}
}

void AGCFPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);


	// TODO: Consider extracting this camera target replication logic into a dedicated component.
	AGCFPlayerState* GCFPlayerState = GetGCFPlayerState();

	if (PlayerCameraManager && GCFPlayerState)
	{
		APawn* TargetPawn = PlayerCameraManager->GetViewTargetPawn();

		if (TargetPawn)
		{
			// Update view rotation on the server so it replicates
			if (HasAuthority() || TargetPawn->IsLocallyControlled())
			{
				GCFPlayerState->SetReplicatedViewRotation(TargetPawn->GetViewRotation());
			}

			// Update the target view rotation if the pawn isn't locally controlled
			if (!TargetPawn->IsLocallyControlled())
			{
				GCFPlayerState = TargetPawn->GetPlayerState<AGCFPlayerState>();
				if (GCFPlayerState)
				{
					// Get it from the spectated pawn's player state, which may not be the same as the PC's playerstate
					TargetViewRotation = GCFPlayerState->GetReplicatedViewRotation();
				}
			}
		}
	}
}

AGCFPlayerState* AGCFPlayerController::GetGCFPlayerState() const
{
	return CastChecked<AGCFPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UGCFAbilitySystemComponent* AGCFPlayerController::GetGCFAbilitySystemComponent() const
{
	const AGCFPlayerState* GCFPS = GetGCFPlayerState();
	return (GCFPS ? GCFPS->GetGCFAbilitySystemComponent() : nullptr);
}

AGCFHUD* AGCFPlayerController::GetGCFHUD() const
{
	return CastChecked<AGCFHUD>(GetHUD(), ECastCheckedType::NullAllowed);
}


void AGCFPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	ConditionalBroadcastTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

void AGCFPlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void AGCFPlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();

	// Unbind from the old player state, if any
	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (IGCFTeamAgentInterface* PlayerStateTeamInterface = Cast<IGCFTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().RemoveAll(this);
		}
	}

	// Bind to the new player state, if any
	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (IGCFTeamAgentInterface* PlayerStateTeamInterface = Cast<IGCFTeamAgentInterface>(PlayerState))
		{
			NewTeamID = PlayerStateTeamInterface->GetGenericTeamId();
			PlayerStateTeamInterface->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
		}
	}

	// Broadcast the team change (if it really has)
	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);

	LastSeenPlayerState = PlayerState;
}

void AGCFPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AGCFPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AGCFPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();

	// When we're a client connected to a remote server, the player controller may replicate later than the PlayerState and AbilitySystemComponent.
	// However, TryActivateAbilitiesOnSpawn depends on the player controller being replicated in order to check whether on-spawn abilities should
	// execute locally. Therefore once the PlayerController exists and has resolved the PlayerState, try once again to activate on-spawn abilities.
	// On other net modes the PlayerController will never replicate late, so GCFASC's own TryActivateAbilitiesOnSpawn calls will succeed. The handling 
	// here is only for when the PlayerState and ASC replicated before the PC and incorrectly thought the abilities were not for the local player.
	if (GetWorld()->IsNetMode(NM_Client))
	{
		if (AGCFPlayerState* GCFPS = GetPlayerState<AGCFPlayerState>())
		{
			if (UGCFAbilitySystemComponent* GCFASC = GCFPS->GetGCFAbilitySystemComponent())
			{
				GCFASC->RefreshAbilityActorInfo();
				GCFASC->TryActivateAbilitiesOnSpawn();
			}
		}
	}

	if (PlayerState) {
		if (UGCFLocalPlayer* LocalPlayer = Cast<UGCFLocalPlayer>(Player)) {
			LocalPlayer->OnPlayerStateSet.Broadcast(LocalPlayer, PlayerState);
		}
	}

	if (PossessionContextComponent) {
		PossessionContextComponent->CheckAndUpdatePlayerPossessionState();
	}
}


void AGCFPlayerController::OnRep_Pawn()
{
}


void AGCFPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	// TODO: Implement user settings initialization (e.g., binding to OnSettingsChanged).
	/*if (const UGCFLocalPlayer* GCFLocalPlayer = Cast<UGCFLocalPlayer>(InPlayer))
	{
		UGCFSettingsShared* UserSettings = GCFLocalPlayer->GetSharedSettings();
		UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);
		OnSettingsChanged(UserSettings);
	}*/
}


void AGCFPlayerController::AddCheats(bool bForce)
{
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else //#if USING_CHEAT_MANAGER
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
}

void AGCFPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogGCF, Warning, TEXT("ServerCheat: %s"), *Msg);
		ClientMessage(ConsoleCommand(Msg));
	}
#endif // #if USING_CHEAT_MANAGER
}

bool AGCFPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AGCFPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogGCF, Warning, TEXT("ServerCheatAll: %s"), *Msg);
		for (TActorIterator<AGCFPlayerController> It(GetWorld()); It; ++It)
		{
			AGCFPlayerController* GCFPC = (*It);
			if (GCFPC)
			{
				GCFPC->ClientMessage(GCFPC->ConsoleCommand(Msg));
			}
		}
	}
#endif // #if USING_CHEAT_MANAGER
}

bool AGCFPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}

void AGCFPlayerController::PreProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PreProcessInput(DeltaTime, bGamePaused);
}

void AGCFPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UGCFAbilitySystemComponent* GCFASC = GetGCFAbilitySystemComponent())
	{
		GCFASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	// Forward input processing to the Pawn's Ability System Component as well.
	if (UGCFAbilitySystemComponent* GCFASC = UGCFAbilitySystemFunctionLibrary::GetPawnAbilitySystemComponent<UGCFAbilitySystemComponent>(GetPawn())) 
	{
		GCFASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}


void AGCFPlayerController::OnPossess(APawn* APawn)
{
	Super::OnPossess(APawn);

	// Set the ViewTarget on the Server side.
	if (IsLocalController()) {
		UE_LOG(LogGCFSystem, Log,
			   TEXT("PC SetViewTarget = %s  NetRole=%s"),
			   *GetWorld()
			   ->GetFirstPlayerController<APlayerController>()
			   ->PlayerCameraManager
			   ->GetViewTarget()
			   ->GetName(),
			   *GetClientServerContextString(this)
		);
		SetViewTarget(APawn);
	}

	if (UGCFLocalPlayer* LocalPlayer = Cast<UGCFLocalPlayer>(Player)) {
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, APawn);
	}

#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (APawn != nullptr) && (GetPawn() == APawn)) {
		for (const FGCFCheatToRun& CheatRow : GetDefault<UGCFDeveloperSettings>()->CheatsToRun) {
			if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession) {
				ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
			}
		}
	}
#endif
}


void AGCFPlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (APawn* PawnBeingUnpossessed = GetPawn()) {
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState)) {
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed) {
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();

	if (UGCFLocalPlayer* LocalPlayer = Cast<UGCFLocalPlayer>(Player)) {
		LocalPlayer->OnPlayerPawnSet.Broadcast(LocalPlayer, nullptr);
	}
}


void AGCFPlayerController::AcknowledgePossession(class APawn* P)
{
	// Call Super first to ensure PC->GetPawn() correctly points to the new InPawn.
	Super::AcknowledgePossession(P);

	if (ControllerPossessionComponent) {
		ControllerPossessionComponent->HandleNewPawn(P);
	}

	// Set the ViewTarget on the Client side.
	if (IsLocalController()) {
		UE_LOG(LogGCFSystem, Log,
			   TEXT("PC SetViewTarget = %s  NetRole=%s"),
			   *GetWorld()
			   ->GetFirstPlayerController<APlayerController>()
			   ->PlayerCameraManager
			   ->GetViewTarget()
			   ->GetName(),
			   *GetClientServerContextString(this)
		);
		SetViewTarget(GetPawn());
	}
}


void AGCFPlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			if (GCF::Input::ShouldAlwaysPlayForceFeedback || CurrentInputType == ECommonInputType::Gamepad || CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}
		}
	}
	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}

void AGCFPlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);
}

void AGCFPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(LogGCFPlayer, Error, TEXT("You can't set the team ID on a player controller (%s); it's driven by the associated player state"), *GetPathNameSafe(this));
}

FGenericTeamId AGCFPlayerController::GetGenericTeamId() const
{
	if (const IGCFTeamAgentInterface* PSWithTeamInterface = Cast<IGCFTeamAgentInterface>(PlayerState))
	{
		return PSWithTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

FOnGCFTeamIndexChangedDelegate* AGCFPlayerController::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}


void AGCFPlayerController::GCF_DumpInputBindings()
{
	if (UGCFInputBindingManagerComponent* Manager = FindComponentByClass<UGCFInputBindingManagerComponent>()) {
		Manager->DumpInputBindings();
	} else {
		UE_LOG(LogCore, Error, TEXT("GCF_DumpInputBindings: Manager Component not found!"));
	}
}


FDelegateHandle AGCFPlayerController::RegisterAndExecuteDelegate(const FOnInputComponentReady::FDelegate& Delegate, bool bExecuteImmediately)
{
	if (bExecuteImmediately) {
		Delegate.ExecuteIfBound(InputComponent);
	}
	return OnInputComponentReady.Add(Delegate);
}


void AGCFPlayerController::RemoveDelegate(const FDelegateHandle& Handle)
{
	OnInputComponentReady.Remove(Handle);
}


void AGCFPlayerController::OnCameraModeMessageReceived(FGameplayTag Channel, const FGCFPolicyChangedCursorMessage& Message)
{
	if (Message.Controller == this) {
		SetShowMouseCursor(Message.bShowCursor);
	}
}