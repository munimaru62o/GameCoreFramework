// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

#pragma once

#include "Engine/World.h"
#include "GCFShared.h"
#include "Movement/GCFMovementTypes.h"
#include "GameplayTagContainer.h"

#include "GCFCameraMode.generated.h"

#define UE_API GAMECOREFRAMEWORK_API

class AActor;
class UCanvas;
class UGCFCameraComponent;

/**
 * EGCFCameraModeBlendFunction
 *
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EGCFCameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT	UMETA(Hidden)
};


/**
 * FGCFCameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FGCFCameraModeView
{
public:

	FGCFCameraModeView();

	void Blend(const FGCFCameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};

/**
 * @brief Data that defines how the camera interacts with the gameplay systems.
 * Broadcasted via GameplayMessages when the camera mode changes.
 */
USTRUCT(BlueprintType)
struct FGCFCameraPolicyData
{
	GENERATED_BODY()

	/** Tag identifying the camera type (e.g., Camera.Mode.ThirdPerson). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "Camera.Mode"))
	FGameplayTag CameraTypeTag;

	/** Whether the mouse cursor should be visible in this mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShowCursor = false;

	/** How movement input and character rotation should be handled (e.g., FreeLook vs Locked). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EGCFMovementRotationPolicy RotationPolicy = EGCFMovementRotationPolicy::CameraDriven;
};


/**
 * @brief Base class for all camera modes.
 *
 * A CameraMode calculates the 'View' (Location, Rotation, FOV) for a frame.
 * Modes are stacked in the CameraComponent and blended together.
 */
UCLASS(MinimalAPI, Abstract, NotBlueprintable)
class UGCFCameraMode : public UObject
{
	GENERATED_BODY()

public:

	UE_API UGCFCameraMode();

	UE_API UGCFCameraComponent* GetGCFCameraComponent() const;

	UE_API virtual UWorld* GetWorld() const override;

	UE_API AActor* GetTargetActor() const;

	const FGCFCameraModeView& GetCameraModeView() const { return View; }

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation() {};

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation() {};

	UE_API void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	UE_API void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const { return PolicyData.CameraTypeTag; }
	const FGCFCameraPolicyData& GetPolicyData() const { return PolicyData; }

	UE_API virtual void DrawDebug(UCanvas* Canvas) const;

protected:
	/** Calculates the pivot point (target location) for the camera. Handles crouching offsets. */
	UE_API virtual FVector GetPivotLocation() const;

	/** Calculates the pivot rotation (usually ControlRotation). */
	UE_API virtual FRotator GetPivotRotation() const;

	/** Main update logic. Should be overridden by subclasses to apply offsets. */
	UE_API virtual void UpdateView(float DeltaTime);

	/** Updates the blend alpha and weight. */
	UE_API virtual void UpdateBlending(float DeltaTime);

	/** helper to get current zoom ratio from the component. */
	UE_API virtual float GetCameraZoomRatio() const;

	/** Helper to evaluate the offset curves based on current Pitch. */
	UE_API FVector CalculateOffsetFromCurves(float Pitch) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGCFCameraPolicyData PolicyData;

	// View output produced by the camera mode.
	FGCFCameraModeView View;

	// The horizontal field of view (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	// Minimum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	// Maximum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	// How long it takes to blend in this mode.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// Function used for blending.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EGCFCameraModeBlendFunction BlendFunction;

	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Linear blend alpha used to determine the blend weight.
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	float BlendWeight;

	/** 
	 * [Helper] Curve for camera offset based on Pitch.
	 * Intended to be used by subclasses (e.g. ThirdPersonMode).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Offset", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetX;

	UPROPERTY(EditDefaultsOnly, Category = "Offset", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetY;

	UPROPERTY(EditDefaultsOnly, Category = "Offset", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetZ;

protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(transient)
	uint32 bResetInterpolation:1;
};

#undef UE_API
