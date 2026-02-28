// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "GCFGameplayAbilitySystemTypes.h"
#include "GCFGameplayAbility.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;

class AActor;
class AController;
class AGCFCharacter;
class AGCFPlayerController;
class APlayerController;
class FText;
class IGCFAbilitySourceInterface;
class UAnimMontage;
class UGCFAbilityCost;
class UGCFAbilitySystemComponent;
class UGCFCameraMode;
class UGCFHeroComponent;
class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEffectSpec;
struct FGameplayEventData;


USTRUCT(BlueprintType)
struct FGCFAbilitySimpleFailureMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	FText UserFacingReason;
};


/** Failure reason that can be used to play an animation montage when a failure occurs */
USTRUCT(BlueprintType)
struct FGCFAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	// Player controller that failed to activate the ability, if the AbilitySystemComponent was player owned
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// Avatar actor that failed to activate the ability
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> AvatarActor = nullptr;

	// All the reasons why this ability has failed
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

/**
 * UGCFGameplayAbility
 *
 *	The base gameplay ability class.
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class."))
class GAMECOREFRAMEWORK_API UGCFGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UGCFAbilitySystemComponent;

public:

	UGCFGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "GCF|Ability")
	UGCFAbilitySystemComponent* GetGCFAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "GCF|Ability")
	AController* GetControllerFromActorInfo() const;
	template <typename T>
	T* GetControllerFromActorInfo() const { return Cast<T>(GetControllerFromActorInfo());}

	UFUNCTION(BlueprintCallable, Category = "GCF|Ability")
	AActor* GetAvatarFromActorInfo() const;
	template <typename T>
	T* GetAvatarFromActorInfo() const { return Cast<T>(GetAvatarFromActorInfo()); }

	EGCFAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EGCFAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
//
//	// Returns true if the requested activation group is a valid transition.
//	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GCF|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
//	bool CanChangeActivationGroup(EGCFAbilityActivationGroup NewGroup) const;
//
//	// Tries to change the activation group.  Returns true if it successfully changed.
//	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GCF|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
//	bool ChangeActivationGroup(EGCFAbilityActivationGroup NewGroup);
//
//	// Sets the ability's camera mode.
//	UFUNCTION(BlueprintCallable, Category = "GCF|Ability")
//	void SetCameraMode(TSubclassOf<UGCFCameraMode> CameraMode);
//
//	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
//	UFUNCTION(BlueprintCallable, Category = "GCF|Ability")
//	void ClearCameraMode();
//
	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		ScriptOnAbilityFailedToActivate(FailedReason);
	}

protected:

	// Called when the ability fails to activate
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	// Called when the ability fails to activate
	UFUNCTION(BlueprintImplementableEvent)
	void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;
//
//	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
//	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
//	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
//	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
//	//~End of UGameplayAbility interface
//
	virtual void OnPawnAvatarSet();

	virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IGCFAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();

protected:

	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Ability Activation")
	EGCFAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GCF|Ability Activation")
	EGCFAbilityActivationGroup ActivationGroup;

	//// Additional costs that must be paid to activate this ability
	//UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	//TArray<TObjectPtr<UGCFAbilityCost>> AdditionalCosts;

	// Map of failure tags to simple error messages
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	// Map of failure tags to anim montages that should be played with them
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;
//
//	// If true, extra information should be logged when this ability is canceled. This is temporary, used for tracking a bug.
//	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
//	bool bLogCancelation;
//
//	// Current camera mode set by the ability.
//	TSubclassOf<UGCFCameraMode> ActiveCameraMode;

};
