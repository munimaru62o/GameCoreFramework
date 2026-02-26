// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#include "Actor/Character/GCFCharacter.h"

#include "GCFShared.h"
#include "AbilitySystem/GCFAbilitySystemComponent.h"
#include "System/Lifecycle/GCFPawnExtensionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Movement/GCFCharacterMovementComponent.h"
#include "Actor/Avatar/GCFAvatarControlComponent.h"
#include "System/Lifecycle/GCFPawnReadyStateComponent.h"
#include "Camera/GCFCameraComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFCharacter)

class AActor;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UInputComponent;

static FName NAME_GCFCharacterCollisionProfile_Capsule(TEXT("GCFPawnCapsule"));
static FName NAME_GCFCharacterCollisionProfile_Mesh(TEXT("GCFPawnMesh"));

AGCFCharacter::AGCFCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCFCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Avoid ticking characters if possible.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetNetCullDistanceSquared(900000000.0f);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(NAME_GCFCharacterCollisionProfile_Capsule);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));  // Rotate mesh to be X forward since it is exported as Y forward.
	MeshComp->SetCollisionProfileName(NAME_GCFCharacterCollisionProfile_Mesh);

	UGCFCharacterMovementComponent* GCFMoveComp = CastChecked<UGCFCharacterMovementComponent>(GetCharacterMovement());
	GCFMoveComp->GravityScale = 1.0f;
	GCFMoveComp->MaxAcceleration = 2400.0f;
	GCFMoveComp->BrakingFrictionFactor = 1.0f;
	GCFMoveComp->BrakingFriction = 6.0f;
	GCFMoveComp->GroundFriction = 8.0f;
	GCFMoveComp->BrakingDecelerationWalking = 1400.0f;
	GCFMoveComp->bUseControllerDesiredRotation = false;
	GCFMoveComp->bOrientRotationToMovement = false;
	GCFMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GCFMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	GCFMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	GCFMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	GCFMoveComp->SetCrouchedHalfHeight(65.0f);

	CameraComponent = CreateDefaultSubobject<UGCFCameraComponent>(TEXT("GCFCameraComponent"));
	CameraComponent->SetupAttachment(CapsuleComp);
	CameraComponent->bUsePawnControlRotation = false;

	PawnExtComponent = CreateDefaultSubobject<UGCFPawnExtensionComponent>(TEXT("PawnExtensionComponent"));

	PawnReadyStateComponent = CreateDefaultSubobject<UGCFPawnReadyStateComponent>(TEXT("PawnReadyStateComponent"));
	AvatarControlComponent = CreateDefaultSubobject<UGCFAvatarControlComponent>(TEXT("AvatarControlComponent"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}


void AGCFCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}


void AGCFCharacter::BeginPlay()
{
	Super::BeginPlay();
}


void AGCFCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


void AGCFCharacter::Reset()
{
	DisableMovementAndCollision();

	K2_OnReset();

	UninitAndDestroy();
}


void AGCFCharacter::NotifyControllerChanged()
{
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Update our team ID based on the controller
	if (HasAuthority() && (GetController() != nullptr)) {
		if (IGCFTeamAgentInterface* ControllerWithTeam = Cast<IGCFTeamAgentInterface>(GetController())) {
			MyTeamID = ControllerWithTeam->GetGenericTeamId();
			ConditionalBroadcastTeamChanged(this, OldTeamId, MyTeamID);
		}
	}
}


FVector AGCFCharacter::GetPawnViewLocation() const
{
	// Get the base eye height defined in the class.
	float TargetEyeHeight = BaseEyeHeight;

	// If using the traditional CharacterMovementComponent, check its crouch state.
	if (const UCharacterMovementComponent* MoveComp = GetCharacterMovement()) {
		if (MoveComp->IsCrouching()) {
			// CMC has a built-in property for crouched eye height, but if you want it to be 
			// physically accurate to the capsule difference, you can calculate it like this:
			const float DefaultHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
			const float CrouchOffset = DefaultHalfHeight - MoveComp->GetCrouchedHalfHeight();

			TargetEyeHeight -= CrouchOffset;
		}
	}

	// Return the actor's world location offset by the calculated eye height.
	return GetActorLocation() + (FVector::UpVector * TargetEyeHeight);
}


void AGCFCharacter::HandleMoveInput_Implementation(const FVector2D& InputValue, const FRotator& MovementRotation)
{
	if (InputValue.X != 0.0f) {
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(MovementDirection, InputValue.X);
	}

	if (InputValue.Y != 0.0f) {
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(MovementDirection, InputValue.Y);
	}
}


void AGCFCharacter::HandleMoveUpInput_Implementation(float Value)
{
	//AddMovementInput(FVector::UpVector, Value);
}


void AGCFCharacter::PossessedBy(AController* NewController)
{
	const FGenericTeamId OldTeamID = MyTeamID;

	Super::PossessedBy(NewController);

	if (PawnExtComponent) {
		PawnExtComponent->OnControllerAssigned();
	}

	// Grab the current team ID and listen for future changes
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(NewController)) {
		MyTeamID = ControllerAsTeamProvider->GetGenericTeamId();
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}

void AGCFCharacter::UnPossessed()
{
	AController* const OldController = GetController();

	// Stop listening for changes from the old controller
	const FGenericTeamId OldTeamID = MyTeamID;
	if (IGCFTeamAgentInterface* ControllerAsTeamProvider = Cast<IGCFTeamAgentInterface>(OldController)) {
		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();

	// Determine what the new team ID should be afterwards
	MyTeamID = DetermineNewTeamAfterPossessionEnds(OldTeamID);
	ConditionalBroadcastTeamChanged(this, OldTeamID, MyTeamID);
}


void AGCFCharacter::OnRep_Controller()
{
	if (PawnExtComponent) {
		PawnExtComponent->OnControllerAssigned();
	}
}


void AGCFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnExtComponent->SetupPlayerInputComponent();
}



void AGCFCharacter::OnDeathStarted(AActor*)
{
	DisableMovementAndCollision();
}

void AGCFCharacter::OnDeathFinished(AActor*)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}


void AGCFCharacter::DisableMovementAndCollision()
{
	if (GetController()) {
		GetController()->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UCharacterMovementComponent* MoveComp = CastChecked<UCharacterMovementComponent>(GetCharacterMovement());
	MoveComp->StopMovementImmediately();
	MoveComp->DisableMovement();
}


void AGCFCharacter::DestroyDueToDeath()
{
	K2_OnDeathFinished();
	UninitAndDestroy();
}


void AGCFCharacter::UninitAndDestroy()
{
	if (GetLocalRole() == ROLE_Authority) {
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	SetActorHiddenInGame(true);
}


// --- IGCFAvatarActionHandler Implementation ---
void AGCFCharacter::HandleJumpInput_Implementation(bool bIsPressed)
{
	if (bIsPressed) {
		Jump(); // ACharacter native method sets bPressedJump = true
	} else {
		StopJumping(); // ACharacter native method sets bPressedJump = false
	}
}


void AGCFCharacter::HandleCrouchInput_Implementation(bool bIsPressed)
{
	// Execute the toggle logic only on the exact frame the button is pressed (Just Pressed).
	if (bIsPressed && !bIsCrouchButtonPressed) {
		const UCharacterMovementComponent* GCFMoveComp = CastChecked<UCharacterMovementComponent>(GetCharacterMovement());

		if (IsCrouched() || GCFMoveComp->bWantsToCrouch) {
			UnCrouch();
		} else if (GCFMoveComp->IsMovingOnGround()) {
			Crouch();
		}
	}

	// Cache the physical button state for the next frame's comparison.
	bIsCrouchButtonPressed = bIsPressed;

	/*
	 * NOTE: If you want to implement "Hold-to-Crouch" instead of "Toggle"
	 * for this legacy character, simply replace the above logic with the following:
	 *
	 * if (bIsPressed) {
	 * Crouch();
	 * } else {
	 * UnCrouch();
	 * }
	 */
}


bool AGCFCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}


void AGCFCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
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


FGenericTeamId AGCFCharacter::GetGenericTeamId() const
{
	return MyTeamID;
}


FOnGCFTeamIndexChangedDelegate* AGCFCharacter::GetOnTeamIndexChangedDelegate()
{
	return &OnTeamChangedDelegate;
}


void AGCFCharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	const FGenericTeamId MyOldTeamID = MyTeamID;
	MyTeamID = IntegerToGenericTeamId(NewTeam);
	ConditionalBroadcastTeamChanged(this, MyOldTeamID, MyTeamID);
}