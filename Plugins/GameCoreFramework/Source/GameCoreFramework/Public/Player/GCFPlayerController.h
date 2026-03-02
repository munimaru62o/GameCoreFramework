// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "ModularPlayerController.h"
#include "Actor/GCFTeamAgentInterface.h"
#include "Input/GCFInputTypes.h"
#include "Messages/GCFMessageTypes.h"
#include "GCFPlayerController.generated.h"

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
class UGCFLocomotionDirectionComponent;
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
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class GAMECOREFRAMEWORK_API AGCFPlayerController : public AModularPlayerController, public IGCFTeamAgentInterface
{
	GENERATED_BODY()

public:

	AGCFPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetupInputComponent() override;

	virtual void SetPawn(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "GCF|PlayerController")
	AGCFPlayerState* GetGCFPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|PlayerController")
	UGCFAbilitySystemComponent* GetGCFAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|PlayerController")
	AGCFHUD* GetGCFHUD() const;

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheatAll(const FString& Msg);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~AController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void AcknowledgePossession(class APawn* P);
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Pawn() override;
	//~End of AController interface

	//~APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetPlayer(UPlayer* InPlayer) override;
	virtual void AddCheats(bool bForce) override;
	virtual void UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId) override;
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface
	
	//~IGCFTeamAgentInterface interface
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual FOnGCFTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of IGCFTeamAgentInterface interface

	FDelegateHandle RegisterAndExecuteDelegate(const FOnInputComponentReady::FDelegate& Delegate, bool bExecuteImmediately = true);
	void RemoveDelegate(const FDelegateHandle& Handle);

	/**
	 * Console command: GCF.DumpInputBindings
	 * Outputs the current input binding state to the log for debugging purposes.
	 */
	UFUNCTION(Exec)
	void GCF_DumpInputBindings();

protected:
	// Called when the player state is set or cleared
	virtual void OnPlayerStateChanged();

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