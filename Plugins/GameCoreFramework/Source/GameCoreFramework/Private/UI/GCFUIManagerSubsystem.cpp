// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/GCFUIManagerSubsystem.h"

#include "CommonLocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "GameUIPolicy.h"
#include "PrimaryGameLayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GCFUIManagerSubsystem)

class FSubsystemCollectionBase;

UGCFUIManagerSubsystem::UGCFUIManagerSubsystem()
{}

void UGCFUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UGCFUIManagerSubsystem::Tick), 0.0f);
}

void UGCFUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

bool UGCFUIManagerSubsystem::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();

	return true;
}

void UGCFUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
	if (const UGameUIPolicy* Policy = GetCurrentUIPolicy()) {
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers()) {
			bool bShouldShowUI = true;

			if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld())) {
				const AHUD* HUD = PC->GetHUD();

				if (HUD && !HUD->bShowHUD) {
					bShouldShowUI = false;
				}
			}

			if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer))) {
				const ESlateVisibility DesiredVisibility = bShouldShowUI ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
				if (DesiredVisibility != RootLayout->GetVisibility()) {
					RootLayout->SetVisibility(DesiredVisibility);
				}
			}
		}
	}
}
