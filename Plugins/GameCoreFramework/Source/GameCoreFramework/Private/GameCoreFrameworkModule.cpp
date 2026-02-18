// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 62o. All rights reserved.

#include "GameCoreFrameworkModule.h"
#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FGameCoreFrameworkModule"

void FGameCoreFrameworkModule::StartupModule()
{
	// 
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GameCoreFramework"));
	if (Plugin.IsValid()) {
		FString BaseDir = Plugin->GetBaseDir();
		FString TagsDir = FPaths::Combine(BaseDir, TEXT("Config"), TEXT("Tags"));
		UGameplayTagsManager::Get().AddTagIniSearchPath(TagsDir);
	}
}

void FGameCoreFrameworkModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGameCoreFrameworkModule, GameCoreFramework)