#include "Data/CombatGameplayTags.h"

namespace CombatGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack, "Combat.Ability.Attack", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_Activate, "Combat.Ability.Attack.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_Abort, "Combat.Ability.Attack.Event.Abort", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_RequestNextAttack, "Combat.Ability.Attack.Event.Continue", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_Reactivate, "Combat.Ability.Attack.Event.Reactivate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_Release, "Combat.Ability.Attack.Event.Release", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_Feint, "Combat.Ability.Attack.Event.Feint", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Attack_Event_CostUnaffordable, "Combat.Ability.Attack.Event.CostUnaffordable", "");
		
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_WeaponType_TwoHanded, "Combat.WeaponType.TwoHanded", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_WeaponType_Saber, "Combat.WeaponType.Saber", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_WeaponType_OneHandedSword, "Combat.WeaponType.OneHandedSword", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_WeaponType_SwordAndShield, "Combat.WeaponType.SwordAndShield", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_State_Stagger, "Combat.State.Stagger", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_State_GuardBreak, "Combat.State.GuardBreak", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_State_Knockdown, "Combat.State.Knockdown", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_HitDirection_Front, "Combat.HitDirection.Front", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_HitDirection_Left, "Combat.HitDirection.Left", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_HitDirection_Right, "Combat.HitDirection.Right", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_HitDirection_Back, "Combat.HitDirection.Back", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Hit_Steel, "Combat.FX.Hit.Steel", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Hit_Environment, "Combat.FX.Hit.Environment", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Hit_Body, "Combat.FX.Hit.Body", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Sound_Grunt, "Combat.FX.Sound.Grunt", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Sound_Hurt, "Combat.FX.Sound.Hurt", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Sound_Staggered, "Combat.FX.Sound.Staggered", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Sound_Dying, "Combat.FX.Sound.Dying", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_FX_Sound_Whoosh, "Combat.FX.Sound.Whoosh", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Death_Event_Activate, "Combat.Ability.Death.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_HitReact_Event_Activate, "Combat.Ability.HitReact.Event.Activate", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Stagger_Event_Activate, "Combat.Ability.Stagger.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Stagger_Event_Abort, "Combat.Ability.Stagger.Event.Abort", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_ReceiveGuardBreak_Event_Activate, "Combat.Ability.ReceiveGuardBreak.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_ReceiveGuardBreak_Event_Abort, "Combat.Ability.ReceiveGuardBreak.Event.Activate", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Knockdown_Event_Activate, "Combat.Ability.Knockdown.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Knockdown_Event_Abort, "Combat.Ability.Knockdown.Event.Abort", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Dodge_Event_Activate, "Combat.Ability.Dodge.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Dodge_Event_Abort, "Combat.Ability.Dodge.Event.Abort", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Clash_Event_Activate, "Combat.Ability.Clash.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Clash_Event_Finished, "Combat.Ability.Clash.Event.Finished", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Clash_Cause_Parried, "Combat.Ability.Clash.Cause.Parried", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_ChargeIn_Event_Activate, "Combat.Ability.ChargeIn.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_ChargeIn_Event_Abort, "Combat.Ability.ChargeIn.Event.Abort", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Block_Event_Activate, "Combat.Ability.Block.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Ability_Block_Event_Stop, "Combat.Ability.Block.Event.Stop", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_SetByCaller_Damage_Health, "SetByCaller.Attack.Damage.Health", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_SetByCaller_Damage_Poise, "SetByCaller.Attack.Damage.Poise", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_SetByCaller_Block_Consumption_Stamina, "SetByCaller.Block.Consumption.Stamina", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_SetByCaller_Block_Consumption_Poise, "SetByCaller.Block.Consumption.Poise", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Combat_Movement_Lock_Attack, "Combat.Movement.Lock.Attack", "");
}