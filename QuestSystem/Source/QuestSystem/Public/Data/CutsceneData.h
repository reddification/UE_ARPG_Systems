#pragma once

class ULevelSequencePlayer;

struct FActiveCusceneData
{
	TWeakObjectPtr<ULevelSequencePlayer> ActiveCutscenePlayer; 
	bool bUnskippable = false;
	bool bPlayerLocked = false;
};
