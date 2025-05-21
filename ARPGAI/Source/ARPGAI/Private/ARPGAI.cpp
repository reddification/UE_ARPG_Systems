// Copyright Epic Games, Inc. All Rights Reserved.

#include "ARPGAI.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FARPGAIModule"

void FARPGAIModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	TSharedPtr<IPlugin> ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("ARPGAI"));
	check(ThisPlugin.IsValid());
	
	UGameplayTagsManager::Get().AddTagIniSearchPath(ThisPlugin->GetBaseDir() / TEXT("Config"));
}

void FARPGAIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FARPGAIModule, ARPGAI)