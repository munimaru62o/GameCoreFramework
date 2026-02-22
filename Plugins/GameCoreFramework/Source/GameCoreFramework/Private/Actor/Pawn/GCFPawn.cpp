// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Actor/Pawn/GCFPawn.h"
#include "Actor/Data/GCFPawnData.h"
#include "Camera/GCFCameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "Input/GCFPawnInputBridgeComponent.h"
#include "MoverComponent.h"

const FName AGCFPawn::CollisionComponentName(TEXT("CollisionComponent"));
const FName AGCFPawn::MeshComponentName(TEXT("MeshComponent"));


AGCFPawn::AGCFPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NetDormancy = DORM_Never;
	bReplicates = true;

	// Create the collision sphere and set it as Root.
	// This enables basic collision detection for floating movement.
	CollisionComponent = CreateOptionalDefaultSubobject<USphereComponent>(CollisionComponentName);
	if (USphereComponent* SphereComp = GetSphereComponent()) {
		SphereComp->InitSphereRadius(32.0f);
		SphereComp->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		SphereComp->SetMobility(EComponentMobility::Movable);
		RootComponent = SphereComp;
	}

	MeshComponent = CreateOptionalDefaultSubobject<UStaticMeshComponent>(MeshComponentName);
	if (UStaticMeshComponent* StaticMeshComp = GetStaticMesh()) {
		StaticMeshComp->SetupAttachment(RootComponent);
	}

	CameraComponent = CreateDefaultSubobject<UGCFCameraComponent>(TEXT("GCFCameraComponent"));
	CameraComponent->SetupAttachment(CollisionComponent);
	CameraComponent->bUsePawnControlRotation = false; // Camera rotation is handled by the CameraComponent/Modes.

	PawnExtensionComponent = CreateDefaultSubobject<UGCFPawnExtensionComponent>(TEXT("GCFPawnExtensionComponent"));
	PawnReadyStateComponent = CreateDefaultSubobject<UGCFPawnReadyStateComponent>(TEXT("GCFPawnReadyStateComponent"));
	PawnInputBridgeComponent = CreateDefaultSubobject<UGCFPawnInputBridgeComponent>(TEXT("PawnInputBridgeComponent"));
}


USphereComponent* AGCFPawn::GetSphereComponent() const
{
	return Cast<USphereComponent>(CollisionComponent);
}


UStaticMeshComponent* AGCFPawn::GetStaticMesh() const
{
	return Cast<UStaticMeshComponent>(MeshComponent);
}


const UGCFPawnData* AGCFPawn::GetPawnData() const
{
	if (PawnExtensionComponent) {
		return PawnExtensionComponent->GetPawnData<UGCFPawnData>();
	}
	return nullptr;
}


void AGCFPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


void AGCFPawn::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	// Notify extension component on Server side possession.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnControllerAssigned();
	}

	// Grab the current team ID and listen for future changes
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(NewController)) {
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}


void AGCFPawn::UnPossessed()
{
	AController* const OldController = GetController();

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamID = MyTeamID;
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(OldController)) {
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	// Notify extension component to clean up or reset state.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->HandleControllerChanged();
	}

	// Determine what the new team ID should be afterwards
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}


void AGCFPawn::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Notify extension component on Client side when the Controller reference replicates.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->OnControllerAssigned();
	}
}


void AGCFPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Notify extension component on Client side when the PlayerState reference replicates.
	// This is crucial for initializing AbilitySystem or HUD that depends on PlayerState data.
	if (PawnExtensionComponent) {
		PawnExtensionComponent->HandlePlayerStateReplicated();
	}
}


void AGCFPawn::HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation)
{
	if (bUseMoverComponent) {
		// Cache the input state for the Mover's tick-based polling system
		CachedMoveInput = InputValue;
		CachedMoveRotation = MovementRotation;
		UpdateCachedTargetMovement();
	} else {
		// Traditional immediate input processing (e.g., for FloatingPawnMovement)
		if (InputValue.X != 0.0f) {
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, InputValue.X);
		}

		if (InputValue.Y != 0.0f) {
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, InputValue.Y);
		}
	}
}


void AGCFPawn::HandleMoveUpInput_Implementation(float Value)
{
	if (bUseMoverComponent) {
		// Cache the vertical input state
		CachedUpInput = Value;
		UpdateCachedTargetMovement();
	} else {
		AddMovementInput(FVector::UpVector, Value);
	}
}


void AGCFPawn::UpdateCachedTargetMovement()
{
	// Horizontal Movement Calculation (Applied with Camera/Controller rotation)
	const FVector ForwardDir = CachedMoveRotation.RotateVector(FVector::ForwardVector) * CachedMoveInput.X;
	const FVector RightDir = CachedMoveRotation.RotateVector(FVector::RightVector) * CachedMoveInput.Y;

	// Vertical Movement Calculation (Absolute World Z-Axis)
	const FVector UpDir = FVector::UpVector * CachedUpInput;

	// Composite and store the final intent (The Producer will read this cached value)
	CachedTargetMovement = ForwardDir + RightDir + UpDir;
}


FVector AGCFPawn::GetDesiredMovementVector_Implementation() const
{
	return CachedTargetMovement;
}


void AGCFPawn::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	if (GetController() == nullptr) {
		if (HasAuthority()) {
			const FGenericTeamId OldTeamID = MyTeamID;
			MyTeamID = NewTeamID;
			ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
		} else {
			UE_LOG(LogGCFCharacter, Error, TEXT("You can't set the team ID on a character (%s) except on the authority"), *GetPathNameSafe(this));
		}
	} else {
		UE_LOG(LogGCFCharacter, Error, TEXT("You can't set the team ID on a possessed character (%s); it's driven by the associated controller"), *GetPathNameSafe(this));
	}
}


FGenericTeamId AGCFPawn::GetGenericTeamId() const
{
	return MyTeamID;
}


FOnGCFTeamIndexChangedDelegate* AGCFPawn::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}


void AGCFPawn::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}


FGenericTeamId AGCFPawn::DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
{
	// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
	return FGenericTeamId::NoTeam;
}


void AGCFPawn::OnDeathStarted(AActor*)
{
	// Disable collision and movement here if needed.
	if (CollisionComponent) {
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}


void AGCFPawn::OnDeathFinished(AActor*)
{
	// Detach controller and destroy pawn logic.
	DetachFromControllerPendingDestroy();
	SetLifeSpan(0.1f);
}