// Fill out your copyright notice in the Description page of Project Settings.


#include "MD_GameplayTags.h"

namespace MyDotaTags
{
	UE_DEFINE_GAMEPLAY_TAG(Character_Health, "Character.Health");
	UE_DEFINE_GAMEPLAY_TAG(Character_HealthMax, "Character.HealthMax");
	UE_DEFINE_GAMEPLAY_TAG(Character_Mana, "Character.Mana");
	UE_DEFINE_GAMEPLAY_TAG(Character_ManaMax, "Character.ManaMax");
	
	/** Camera Tags */
	UE_DEFINE_GAMEPLAY_TAG(Camera_FollowingHero, "Camera.FollowingHero");
	
	/** Camera Ability Tags */
	UE_DEFINE_GAMEPLAY_TAG(Ability_Camera_FollowingHero, "Ability.Camera.FollowingHero");
}
