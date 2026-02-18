// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#include "Camera/GCFCameraModeStack.h"
#include "Camera/Mode/GCFCameraMode.h"
#include "Engine/Canvas.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFCameraModeStack)


UGCFCameraModeStack::UGCFCameraModeStack()
{
	bIsActive = false;
}

void UGCFCameraModeStack::ActivateStack()
{
	if (!bIsActive) {
		bIsActive = true;

		// Notify camera modes that they are being activated.
		for (UGCFCameraMode* CameraMode : CameraModeStack) {
			check(CameraMode);
			CameraMode->OnActivation();
		}
	}
}

void UGCFCameraModeStack::DeactivateStack()
{
	if (bIsActive) {
		bIsActive = false;

		// Notify camera modes that they are being deactivated.
		for (UGCFCameraMode* CameraMode : CameraModeStack) {
			check(CameraMode);
			CameraMode->OnDeactivation();
		}
	}
}

void UGCFCameraModeStack::PushCameraMode(TSubclassOf<UGCFCameraMode> CameraModeClass)
{
	if (!CameraModeClass) {
		return;
	}

	UGCFCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	// If already at the top (Index 0), do nothing.
	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode)) {
		return;
	}

	// Check if it already exists in the stack. If so, capture its current contribution and remove it temporarily.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex) {
		if (CameraModeStack[StackIndex] == CameraMode) {
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= CameraMode->GetBlendWeight();
			break;
		} else {
			// Accumulate the inverse weight of modes covering it.
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	if (ExistingStackIndex != INDEX_NONE) {
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	} else {
		ExistingStackContribution = 0.0f;
	}

	// Determine initial blend weight.
	// If it was already in the stack, start from its current visual contribution to avoid popping.
	const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
	const float NewBlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(NewBlendWeight);

	// Insert at the Top (Index 0)
	CameraModeStack.Insert(CameraMode, 0);

	// Ensure the bottom-most mode is always fully opaque (weight 1.0)
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Only trigger activation/notify if it wasn't already in the stack.
	if (ExistingStackIndex == INDEX_NONE) {
		CameraMode->OnActivation();
		OnStackChanged.Broadcast();
	}
}

bool UGCFCameraModeStack::EvaluateStack(float DeltaTime, FGCFCameraModeView& OutCameraModeView)
{
	if (!bIsActive) {
		return false;
	}

	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);

	return true;
}

UGCFCameraMode* UGCFCameraModeStack::GetCameraModeInstance(TSubclassOf<UGCFCameraMode> CameraModeClass)
{
	check(CameraModeClass);

	// Reuse existing instance from pool
	for (UGCFCameraMode* CameraMode : CameraModeInstances) {
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass)) {
			return CameraMode;
		}
	}

	// Create new instance if not found
	UGCFCameraMode* NewCameraMode = NewObject<UGCFCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

void UGCFCameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0) {
		return;
	}

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex) {
		UGCFCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		// If a layer becomes fully opaque (Weight >= 1.0), 
		// any layers below it are completely obscured and can be removed.
		if (CameraMode->GetBlendWeight() >= (1.0f - KINDA_SMALL_NUMBER) ) {
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);
			break;
		}
	}

	if (RemoveCount > 0) {
		for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex) {
			UGCFCameraMode* CameraMode = CameraModeStack[StackIndex];
			check(CameraMode);
			CameraMode->OnDeactivation();
		}

		CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
		OnStackChanged.Broadcast();
	}
}

void UGCFCameraModeStack::BlendStack(FGCFCameraModeView& OutCameraModeView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0) {
		return;
	}

	// Start blending from the bottom (oldest) up to the top.
	const UGCFCameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex) {
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UGCFCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

	for (const UGCFCameraMode* CameraMode : CameraModeStack) {
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UGCFCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	UGCFCameraMode* TopEntry = GetTopStackMode();
	if (TopEntry) {
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	} else {
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag::EmptyTag;
	}
}

UGCFCameraMode* UGCFCameraModeStack::GetTopStackMode() const
{
	return (CameraModeStack.Num() > 0) ? CameraModeStack[0] : nullptr;
}