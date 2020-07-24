// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RepGraphTest : ModuleRules
{
	public RepGraphTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
				"OnlineSubsystemUtils",
				"OnlineSubsystemNull"
			});
		
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate", 
				"SlateCore",
				"OnlineSubsystem",
				"ReplicationGraph"
			});
	}
}
