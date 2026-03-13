// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyDota : ModuleRules
{
	public MyDota(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"NavigationSystem",
			"UMG",
			"Slate",
			"SlateCore",
			"NetCore",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
	}
}
