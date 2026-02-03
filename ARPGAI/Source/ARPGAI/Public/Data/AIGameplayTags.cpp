#include "Data/AIGameplayTags.h"

namespace AIGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Gesture_Death_Front, "AI.Ability.Gesture.Death.Front", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Gesture_Death_Back, "AI.Ability.Gesture.Death.Back", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Gesture_Taunt, "AI.Ability.Gesture.Taunt", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Gesture_Taunt_UpperBody, "AI.Ability.Gesture.UpperBody", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Combat_Combo_MoveFinished, "AI.Event.Combat.MoveFinished", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Combat_TargetUnreachable, "AI.Event.Combat.TargetUnreachable", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_GameplayEffect_Attack_Damage_Health, "SetByCaller.Attack.Damage.Health", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_GameplayEffect_Attack_Damage_Poise, "SetByCaller.Attack.Damage.Poise", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Event_Mob_Killed, "AI.GameplayEvent.Mob.Killed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_AI_Spawn, "GameplayCue.AI.Spawn", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Idle, "AI.Behavior.Idle", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Combat, "AI.Behavior.Combat", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Combat_Engage, "AI.Behavior.Combat.Engage", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Combat_Pursue, "AI.Behavior.Combat.Pursue", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Combat_Retreat, "AI.Behavior.Combat.Retreat", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Combat_Fight, "AI.Behavior.Combat.Fight", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Behavior_Combat_Event_LostContact, "AI.Behavior.Combat.Event.LostContact", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Interaction_Dialogue, "AI.Interaction.Dialogue", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Interaction_Dialogue_Refuse_Hostile, "AI.Interaction.Dialogue.Refuse.Hostile", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Interaction_Dialogue_Refuse_Busy, "AI.Interaction.Dialogue.Refuse.Busy", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Interaction_Dialogue_Refuse_Sleeping, "AI.Interaction.Dialogue.Refuse.Sleeping", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Interaction_Loot, "AI.Interaction.Loot", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Interaction_Steal, "AI.Interaction.Steal", "");

	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Threat_Minor, "AI.Threat.Minor", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Threat_Considerable, "AI.Threat.Considerable", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Threat_Dangerous, "AI.Threat.Dangerous", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Ability_Rotate_Event_Activate, "AI.Ability.Rotate.Event.Activate", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Ability_Rotate_Event_Abort, "AI.Ability.Rotate.Event.Abort", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Ability_Attack_Cooldown, "AI.Ability.Attack.Cooldown", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Noise_Shot, "AI.Noise.Shot", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Noise_EnemySpotted, "AI.Noise.EnemySpotted", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Noise_Throwable_Bounce, "AI.Noise.Throwable.Bounce", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Noise_Footstep, "AI.Noise.Footstep", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Noise_Report_VisualContact_Acquired, "AI.Noise.Report.VisualContact.Acquired", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Noise_Report_VisualContact_Lost, "AI.Noise.Report.VisualContact.Lost", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Npc_Death, "GameplayEvent.Npc.Death", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Calm, "AI.State.Calm", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Combat_Retreating, "AI.State.Combat.Retreat", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Combat_Approach_Menacing, "AI.State.Combat.Approach.Menacing", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Combat_Approach_Anxious, "AI.State.Combat.Approach.Anxious", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Combat_Approach_Calm, "AI.State.Combat.Approach.Calm", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Combat_Fight, "AI.State.Combat.Fight", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Queue, "AI.State.Queue", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Queue_First, "AI.State.Queue.First", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Conversation_OnHold, "AI.State.Conversation.OnHold", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_Conversation_MaintainActivity_IgnoreOrientationToInvoker, "AI.State.Conversation.MaintainActivity.IgnoreOrientationToInvoker", "")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_State_DirectVisualContact, "AI.State.DirectVisualContact", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Ability_ActivationFailed_CantAfford, "AI.BrainMessage.Ability.ActivationFailed.CantAffordCost", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Ability_ActivationFailed_ConditionsNotMet, "AI.BrainMessage.Ability.ActivationFailed.ConditionsNotMet", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_SetWeaponReady_Completed, "AI.BrainMessage.SetWeaponReady.Complete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Dodge_Completed, "AI.BrainMessage.Dodge.Complete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Gesture_Completed, "AI.BrainMessage.Gesture.Complete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Stagger_Completed, "AI.BrainMessage.Stagger.Complete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Block_Completed, "AI.BrainMessage.Block.Complete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Block_ParriedAttack, "AI.BrainMessage.Block.ParriedAttack", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_ChargeIn_Completed, "AI.BrainMessage.ChargeIn.Complete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_LookAt_Completed, "AI.BrainMessage.LookAt.Complete", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_Parried, "AI.BrainMessage.Attack.Parried", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_Whiffed, "AI.BrainMessage.Attack.Whiffed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_HitTarget, "AI.BrainMessage.Attack.HitTarget", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_Started, "AI.BrainMessage.Attack.Started", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_Commited, "AI.BrainMessage.Attack.Commited", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_Completed, "AI.BrainMessage.Attack.Completed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_Canceled, "AI.BrainMessage.Attack.Canceled", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Attack_EnemyBlocking, "AI.BrainMessage.Attack.EnemyBlocking", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Dialogue_Npc_Completed, "AI.BrainMessage.Dialogue.Npc.Completed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Dialogue_Player_Completed, "AI.BrainMessage.Dialogue.Player.Completed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_SmartObjectInteraction_Completed, "AI.BrainMessage.SmartObjectInteraction.Completed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Conversation_OnHold, "AI.BrainMessage.Conversation.OnHold", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Conversation_Completed, "AI.BrainMessage.Conversation.Completed", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_BrainMessage_Backdash_Completed, "AI.BrainMessage.Backdash.Completed", "");
		
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Attitude_Friendly, "AI.Attitude.Friendly", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Attitude_Neutral, "AI.Attitude.Neutral", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Attitude_Cautious, "AI.Attitude.Cautious", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Attitude_Hostile, "AI.Attitude.Hostile", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Attitude_Hostile_Lethal, "AI.Attitude.Hostile.Lethal", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Attitude_Hostile_NonLethal, "AI.Attitude.Hostile.NonLethal", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_ReactionEvaluator_ReactToThreat, "AI.ReactionEvaluator.ReactToThreat", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_ReactionEvaluator_ExecutionResult_Success, "AI.ReactionEvaluator.ExecutionResult.Success", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_ReactionEvaluator_ExecutionResult_Failure, "AI.ReactionEvaluator.ExecutionResult.Failure", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_ReactionEvaluator_ExecutionResult_Abort, "AI.ReactionEvaluator.ExecutionResult.Abort", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_SetByCaller_Time, "AI.SetByCaller.Time", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_SetByCaller_Speed_Min, "AI.SetByCaller.Speed.Min", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_SetByCaller_Speed_Max, "AI.SetByCaller.Speed.Max", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_SetByCaller_Speed_Scale, "AI.SetByCaller.Speed.Scale", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Activity_Goal_Result_SmartObject_NotFound, "AI.Activity.Goal.Result.SmartObject.NotFound", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AI_Activity_Goal_Result_SmartObject_InteractionFinished, "AI.Activity.Goal.Result.SmartObject.InteractionFinished", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Goal_State_StayInQueue_Enter, "AI.Activity.Goal.State.StayInQueue.Enter", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Goal_State_StayInQueue_Finished, "AI.Activity.Goal.State.StayInQueue.Finished", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Goal_Result_Execution_Success, "AI.Activity.Goal.Result.Execution.Success", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Goal_Result_Execution_Failure, "AI.Activity.Goal.Result.Execution.Failure", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Goal_Parameter_LocationId, "AI.Activity.Goal.Parameter.LocationId", "");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Location_Activity, "AI.Location.Activity", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Location_Arbitrary, "AI.Location.Arbitrary", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Location_Spawner, "AI.Location.Spawner", "");
}