// Copyright Epic Games, Inc. All Rights Reserved.
// Portions Copyright (c) 2026 munimaru62o. All rights reserved.

using UnrealBuildTool;

public class GameCoreFramework : ModuleRules
{
	public GameCoreFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "GameplayMessageRuntime",
                "ChaosVehicles",   
				"PhysicsCore",
				"Chaos",
				"Mover",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects",
                "GameplayTags",
                "GameplayAbilities",
                "GameFeatures",
                "ModularGameplay",
                "ModularGameplayActors",
                "EnhancedInput",
                "NetCore",

                "CommonLoadingScreen",
                "CommonUser",
				"CommonUI",
				"CommonGame",
				"CommonInput",
				"GameSubtitles",
				"UMG",
                "CoreOnline",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "DeveloperSettings",
                "AIModule",
                "AudioMixer",
                "EngineSettings",
                "GameplayTasks",
                "UIExtension",
                "ApplicationCore",
				// ... add private dependencies that you statically link with here ...	
            }
            );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        PublicDefinitions.Add("UE_WITH_DTLS=0");
        PublicDefinitions.Add("USING_CHEAT_MANAGER=0");
    }
}
