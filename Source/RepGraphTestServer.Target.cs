// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

/**
 * Build Target for Dedicated Server (Linux)
 * - This does not use Steam
 * - For testing the Dedicated Server
 */
public class RepGraphTestServerTarget : TargetRules
{
	public RepGraphTestServerTarget(TargetInfo Target) : base(Target)
	{
		
		Type = TargetType.Server;
		
		// This is for Dedicated Server use, no steam SDK!
		bUsesSteam = false;
		
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( 
			new string[]
			{
				"RepGraphTest"
			});
	}
}
