// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Camera/GCFCameraComponent.h"
#include "Camera/Mode/GCFCameraMode.h"
#include "Camera/GCFCameraModeStack.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Actor/Pawn/GCFPawn.h"
#include "Actor/Data/GCFPawnData.h"
#include "Actor/Data/GCFPawnDataProvider.h"
#include "Actor/GCFActorFunctionLibrary.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/GCFGameplayMessages.h"
#include "Common/GCFNames.h"
#include "Common/GCFGameplayTags.h"
#include "System/Binder/GCFPawnReadyStateBinder.h"
#include "System/Binder/GCFPawnPossessionBinder.h"

#include "Misc/EnumClassFlags.h"
#include "Camera/PlayerCameraManager.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFCameraComponent)


UGCFCameraComponent::UGCFCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CameraModeStack = nullptr;
	FieldOfViewOffset = 0.0f;
	CachedPawnReadyState = EGCFPawnReadyState::None;
}


void UGCFCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameFrameworkComponentManager* GFCM = UGameFrameworkComponentManager::GetForActor(GetOwner())) {
		if (APawn* Pawn = GetPawn<APawn>()) {
			BinderList.Emplace(FGCFPawnReadyStateBinder::CreateBinder(GFCM, Pawn, FGCFOnPawnReadyStateChangedNative::FDelegate::CreateUObject(this, &ThisClass::HandlePawnReadyStateChanged)));
		}
	}
}


void UGCFCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CameraModeStack) {
		CameraModeStack->DeactivateStack();
	}
	BinderList.Empty();
	StackChangeHandle.Reset();
	Super::EndPlay(EndPlayReason);
}


void UGCFCameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack) {
		CameraModeStack = NewObject<UGCFCameraModeStack>(this);
		check(CameraModeStack);

		// Subscribe to Stack Changes
		FDelegateHandle RawHandle = CameraModeStack->OnStackChanged.AddUObject(this, &ThisClass::HandleCameraModeStackChanged);

		// [Note] Using FGCFDelegateHandle manually here because CameraModeStack is a UObject, not a Component/Actor with standard bind methods.
		// If you standardize CameraModeStack, you could make a helper for this too.
		TWeakObjectPtr<UGCFCameraModeStack> WeakStack(CameraModeStack);
		StackChangeHandle = MakeUnique<FGCFDelegateHandle>([WeakStack, RawHandle]() {
			if (WeakStack.IsValid()) {
				WeakStack->OnStackChanged.Remove(RawHandle);
			}
		});
	}
}


void UGCFCameraComponent::HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot)
{
	static const EGCFPawnReadyState Required = EGCFPawnReadyState::GamePlay | EGCFPawnReadyState::Possessed;
	if (GCF::Bitmask::HasFlagsChanged(Snapshot.State, CachedPawnReadyState, Required)) {
		if (GCF::Bitmask::AreFlagsSet(Snapshot.State, Required)) {
			AttemptInitialize();
			AttemptCameraModeChanged();
		}
	}
	CachedPawnReadyState = Snapshot.State;
}


void UGCFCameraComponent::HandleCameraModeStackChanged()
{
	if (!CameraModeStack) {
		return;
	}

	if (UGCFCameraMode* TopMode = CameraModeStack->GetTopStackMode()) {
		const FGameplayTag NewTag = TopMode->GetCameraTypeTag();
		if (NewTag != LastActiveCameraModeTag) {
			NotifyCameraModeChanged(TopMode->GetPolicyData());
			LastActiveCameraModeTag = NewTag;
		}
	}
}


void UGCFCameraComponent::AttemptInitialize()
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->IsLocallyControlled()) {
		return;
	}

	if (!CameraModeStack || CameraModeStack->IsStackActive()) {
		return;
	}


	UE_LOG(LogGCFSystem, Log,
		   TEXT("ActivateStack: CameraComp=%p World=%s Pawn=%s NetRole=%s"),
		   this,
		   *GetWorld()->GetName(),
		   *GetPawn<APawn>()->GetName(),
		   *GetClientServerContextString(this));

	// Apply Default Camera Mode from PawnData
	CameraModeStack->ActivateStack();
	if (TScriptInterface<IGCFPawnDataProvider> DataProvider = UGCFActorFunctionLibrary::ResolvePawnDataProvider(this)) {
		if (const UGCFPawnData* PawnData = DataProvider->GetPawnData<UGCFPawnData>()) {
			if (PawnData->DefaultCameraMode) {
				CameraModeStack->PushCameraMode(PawnData->DefaultCameraMode);
			}
		}
	}
}


void UGCFCameraComponent::AttemptCameraModeChanged()
{
	if (!CameraModeStack) {
		return;
	}

	if (UGCFCameraMode* TopMode = CameraModeStack->GetTopStackMode()) {
		NotifyCameraModeChanged(TopMode->GetPolicyData());
	}
}


void UGCFCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	// Optimization: Skip heavy calculations on dedicated server or non-local simulated proxies if not needed.
	// (Though UCameraComponent usually handles IsLocallyControlled checks, enforcing it here is safe.)
	if (APawn* Pawn = GetPawn<APawn>()) {
		if (!Pawn->IsLocallyControlled()) {
			Super::GetCameraView(DeltaTime, DesiredView);
			return;
		}
	}

	FGCFCameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	// Apply final view
	CameraModeView.FieldOfView += FieldOfViewOffset;
	FieldOfViewOffset = 0.0f;
	FieldOfView = CameraModeView.FieldOfView;

	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f) {
		DesiredView.PostProcessSettings = PostProcessSettings;
	}

	// In XR much of the camera behavior above is irrellevant, but the post process settings are not.
	if (IsXRHeadTrackedCamera()) {
		Super::GetCameraView(DeltaTime, DesiredView);
	}
}


void UGCFCameraComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("GCFCameraComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), FieldOfView));

	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}


void UGCFCameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(/*out*/ OutWeightOfTopLayer, /*out*/ OutTagOfTopLayer);
}


void UGCFCameraComponent::NotifyCameraModeChanged(const FGCFCameraPolicyData& NewPolicy)
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) {
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) {
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller) {
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(World);
	{
		FGCFCameraModeChangedMessage Message;
		Message.NewCameraModeTag = NewPolicy.CameraTypeTag;
		Message.Controller = Controller;
		MessageSubsystem.BroadcastMessage(GCFGameplayTags::Message_Camera_ModeChange, Message);
	}
	{
		FGCFPolicyChangedCursorMessage Message;
		Message.bShowCursor = NewPolicy.bShowCursor;
		Message.Controller = Controller;
		MessageSubsystem.BroadcastMessage(GCFGameplayTags::Message_PolicyChange_Cursor, Message);
	}
	{
		FGCFPolicyChangedMovementRotationMessage Message;
		Message.NewPolicy = NewPolicy.RotationPolicy;
		Message.Controller = Controller;
		MessageSubsystem.BroadcastMessage(GCFGameplayTags::Message_PolicyChange_MovementRotation, Message);
	}

	UE_LOG(LogTemp, Log, TEXT("GCFCamera: Mode Changed to [%s], Message Broadcasted."), *NewPolicy.CameraTypeTag.ToString());
}