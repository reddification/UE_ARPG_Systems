// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ARPGAI : ModuleRules
{
	public ARPGAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
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
				"AIModule",
				"NavigationSystem",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayAbilities",
				"GameplayTasks",
				"ModularGameplay",
				"ModularGameplayActors",
				"GameplayTags",
				"DeveloperSettings",
				"ModularGameplay",
				"Niagara",
				"Networking",
				"NetCore",
				"UMG",
				"CommonUI",
				"GameFeatureHelpers",
				"GameplayBehaviorsModule", 
				"Projects", "Navmesh",
				"SmartObjectsModule",
				"GameplayBehaviorsModule",
				"GameplayBehaviorSmartObjectsModule",
				"Flow", "FlowGraphUtils" 
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"StructUtils"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
