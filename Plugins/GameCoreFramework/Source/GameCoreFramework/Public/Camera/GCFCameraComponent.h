// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "System/Lifecycle/GCFStateTypes.h"
#include "GameplayTagContainer.h"

#include "GCFCameraComponent.generated.h"

class UCanvas;
class UGCFCameraMode;
class UGCFCameraModeStack;
class UObject;
class FGCFContextBinder;
class FGCFDelegateHandle;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
struct FComponentRequestHandle;
struct FGCFCameraPolicyData;

/**
 * @brief Extended CameraComponent that manages a stack of CameraModes.
 *
 * Supports:
 * - Blending between multiple camera modes (Stack System).
 * - Automatic initialization based on Pawn Ready State.
 * - broadcasting policy changes (Cursor, Rotation) via GameplayMessages.
 */
UCLASS()
class UGCFCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UGCFCameraComponent(const FObjectInitializer& ObjectInitializer);

	/** Helper to cast Owner to specific Pawn type. */
	template <class T>
	T* GetPawn() const
	{
		return Cast<T>(GetOwner());
	}

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "GCF|Camera")
	static UGCFCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UGCFCameraComponent>() : nullptr); }

	// Returns the target actor that the camera is looking at.
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	// Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	virtual void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

    void AddZoomRatio(float DeltaRatio) { TargetZoomRatio = FMath::Clamp(TargetZoomRatio + DeltaRatio, ZoomRatioMin, ZoomRatioMax); }
    float GetTargetZoomRatio() const { return TargetZoomRatio; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	/** Callback when Pawn reaches a new readiness state (e.g. GameplayReady). */
	void HandlePawnReadyStateChanged(const FGCFPawnReadyStateSnapshot& Snapshot);

	/** Callback when the Camera Stack changes (e.g. new mode pushed/popped). */
	void HandleCameraModeStackChanged();

	/** Broadcasts policy changes to other systems. */
	void NotifyCameraModeChanged(const FGCFCameraPolicyData& NewPolicy);

private:
	/** Activates the stack and pushes the default mode. */
	void AttemptInitialize();

	/** Checks the top mode and notifies policy changes if needed. */
	void AttemptCameraModeChanged();

protected:
	// Stack used to blend the camera modes.
	UPROPERTY()
	TObjectPtr<UGCFCameraModeStack> CameraModeStack;

	// Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	float FieldOfViewOffset;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom")
	float TargetZoomRatio	= 1.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom")
	float ZoomRatioMin		= 0.5f;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom")
	float ZoomRatioMax		= 2.0f;

private:
	/** Handles for various state observers. */
	TArray<TUniquePtr<FGCFContextBinder>> BinderList;

	/** Handle for CameraStack delegate. */
	TUniquePtr<FGCFDelegateHandle> StackChangeHandle;

	/** Tracks the last active mode tag to detect changes. */
	UPROPERTY(Transient)
	FGameplayTag LastActiveCameraModeTag;

	EGCFPawnReadyState CachedPawnReadyState;
};
