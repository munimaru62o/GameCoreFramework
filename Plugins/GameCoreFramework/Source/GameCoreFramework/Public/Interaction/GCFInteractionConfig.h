// Copyright (c) 2026 62o. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GCFInteractionConfig.generated.h"

class UGCFInteractionMode;

/**
 * @brief Defines the link between a Camera Mode and the interaction detection logic.
 */
USTRUCT(BlueprintType)
struct FGCFInteractionModeSetting
{
	GENERATED_BODY()

	/** The Camera Mode tag to associate with this logic (e.g., Camera.Mode.ThirdPerson). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "Camera.Mode"))
	FGameplayTag CameraModeTag;

	/** The logic class used to detect interactable objects (e.g., LineTrace vs SphereOverlap). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGCFInteractionMode> InteractionModeClass;

	/** Maximum distance for the detection trace. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float TraceDistance = 1000.0f;
};

/**
 * @brief Data Asset that maps Camera Modes to specific Interaction Modes.
 *
 * Allows the game to switch detection strategies data-drivenly.
 * For example:
 * - ThirdPerson Mode -> Uses "Line Trace from Camera center"
 * - TopDown Mode     -> Uses "Sphere Overlap around Character"
 */
UCLASS(BlueprintType)
class GAMECOREFRAMEWORK_API UGCFInteractionConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TArray<FGCFInteractionModeSetting> ModeSettings;

	/** Helper to find the setting associated with the given Camera Mode tag. */
	const FGCFInteractionModeSetting* FindSettingForCameraMode(FGameplayTag CameraTag) const
	{
		for (const auto& Setting : ModeSettings) {
			if (Setting.CameraModeTag.MatchesTagExact(CameraTag)) return &Setting;
		}
		return nullptr;
	}
};