// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FCombatModule"

void FCombatModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("Combat"));
	check(ThisPlugin.IsValid());
	
	UGameplayTagsManager::Get().AddTagIniSearchPath(ThisPlugin->GetBaseDir() / TEXT("Config"));
}

void FCombatModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCombatModule, Combat)