// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "ModularPlayerController.h"
#include "Actor/GCFTeamAgentInterface.h"
#include "Input/GCFInputTypes.h"
#include "Messages/GCFMessageTypes.h"
#include "GCFPlayerController.generated.h"

#define UE_API GAMECOREFRAMEWORK_API


struct FGenericTeamId;
struct FComponentRequestHandle;

class AGCFHUD;
class AGCFPlayerState;
class APawn;
class APlayerState;
class FPrimitiveComponentId;
class IInputInterface;
class UGCFAbilitySystemComponent;
class UObject;
class UPlayer;
class UGCFPlayerReadyStateComponent;
class UGCFControllerPossessionComponent;
class UGCFInputBindingManagerComponent;
class UGCFPlayerInputBridgeComponent;
class UGCFInputContextComponent;
class UGCFPossessionContextComponent;
class UGCFCameraControlComponent;
class UGCFMovementControlComponent;
class UGCFAbilityInputRouterComponent;

class UMoverNetworkPredictionLiaisonComponent;
class UMoverComponent;

struct FFrame;
struct FGCFPolicyChangedCursorMessage;

/**
 * AGCFPlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class AGCFPlayerController : public AModularPlayerController, public IGCFTeamAgentInterface
{
	GENERATED_BODY()

public:

	UE_API AGCFPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void SetupInputComponent() override;

	UE_API virtual void SetPawn(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "GCF|PlayerController")
	UE_API AGCFPlayerState* GetGCFPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|PlayerController")
	UE_API UGCFAbilitySystemComponent* GetGCFAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|PlayerController")
	UE_API AGCFHUD* GetGCFHUD() const;

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	UE_API void ServerCheatAll(const FString& Msg);

	//~AActor interface
	UE_API virtual void PreInitializeComponents() override;
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~AController interface
	UE_API virtual void OnPossess(APawn* InPawn) override;
	UE_API virtual void OnUnPossess() override;
	UE_API virtual void AcknowledgePossession(class APawn* P);
	UE_API virtual void InitPlayerState() override;
	UE_API virtual void CleanupPlayerState() override;
	UE_API virtual void OnRep_PlayerState() override;
	UE_API virtual void OnRep_Pawn() override;
	//~End of AController interface

	//~APlayerController interface
	UE_API virtual void ReceivedPlayer() override;
	UE_API virtual void PlayerTick(float DeltaTime) override;
	UE_API virtual void SetPlayer(UPlayer* InPlayer) override;
	UE_API virtual void AddCheats(bool bForce) override;
	UE_API virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	UE_API virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	UE_API virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	UE_API virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface
	
	//~IGCFTeamAgentInterface interface
	UE_API virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	UE_API virtual FGenericTeamId GetGenericTeamId() const override;
	UE_API virtual FOnGCFTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGCFTeamAgentInterface interface

	FDelegateHandle RegisterAndExecuteDelegate(const FOnInputComponentReady::FDelegate& Delegate, bool bExecuteImmediately = true);
	void RemoveDelegate(const FDelegateHandle& Handle);

	/**
	 * Console command: GCF.DumpInputBindings
	 * Outputs the current input binding state to the log for debugging purposes.
	 */
	UFUNCTION(Exec)
	UE_API void GCF_DumpInputBindings();

protected:
	// Called when the player state is set or cleared
	UE_API virtual void OnPlayerStateChanged();

private:
	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	void BroadcastOnPlayerStateChanged();

	void OnCameraModeMessageReceived(FGameplayTag Channel, const FGCFPolicyChangedCursorMessage& Message);

private:
	UPROPERTY()
	FOnGCFTeamIndexChangedDelegate OnTeamChangedDelegate;

	FOnInputComponentReady OnInputComponentReady;

	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFControllerPossessionComponent> ControllerPossessionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFInputBindingManagerComponent> InputBindingManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPlayerInputBridgeComponent> PlayerInputBridgeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFInputContextComponent> InputContextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFPossessionContextComponent> PossessionContextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGCFAbilityInputRouterComponent> AbilityInputRouterComponent;

	// -------------------------------------------------------------------
	// [NPP Clock Sync Workaround]
	// Dummy components required to maintain Network Prediction Plugin (NPP) 
	// clock synchronization in a hybrid movement environment (CMC + Mover). 
	// This is officially adopted as a structural adapter until the plugin 
	// natively supports decoupled clock ticking.
	// -------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMoverComponent> MoverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GCF|Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMoverNetworkPredictionLiaisonComponent> MoverLiaisonComponent;

	bool bHideViewTargetPawnNextFrame = false;

	TUniquePtr<FGCFMessageSubscription> MessageHandle;
};


#undef UE_API
