// Fill out your copyright notice in the Description page of Project Settings.

#include "MD_GameplayTags.h"

namespace MyDotaTags
{
	/** InputTags */
	UE_DEFINE_GAMEPLAY_TAG(InputTag, "InputTag");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_FollowToHero, "InputTag.FollowToHero");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack, "InputTag.Attack");

	/** Main stats Tag  */
	UE_DEFINE_GAMEPLAY_TAG(Character_Health, "Character.Health");
	UE_DEFINE_GAMEPLAY_TAG(Character_HealthMax, "Character.HealthMax");
	UE_DEFINE_GAMEPLAY_TAG(Character_Mana, "Character.Mana");
	UE_DEFINE_GAMEPLAY_TAG(Character_ManaMax, "Character.ManaMax");

	/** Status */
	UE_DEFINE_GAMEPLAY_TAG(Status, "Status");
	UE_DEFINE_GAMEPLAY_TAG(Status_Death, "Status.Death");

	/** GameplayCues */
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Character_Respawn, "GameplayCue.Character.Respawn");

	/** Camera Tags */
	UE_DEFINE_GAMEPLAY_TAG(Camera_Locked, "Camera.Locked");
	UE_DEFINE_GAMEPLAY_TAG(Camera_FollowingHero, "Camera.FollowingHero");

	/** Camera Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ShowDraft, "Ability.ShowDraft");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ShowGameplayHUD, "Ability.ShowGameplayHUD");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ShowOverhead, "Ability.ShowOverhead");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Camera_FollowingHero, "Ability.Camera.FollowingHero");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack, "Ability.Attack");

	/** Events */
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Hit, "Event.Montage.Hit");

	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_RequestAttack, "Event.Ability.RequestAttack");
} // namespace MyDotaTags
