// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"


namespace MyDotaTags
{
	/** InputTags */
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_FollowToHero);
	
	/** Main stats Tag  */
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Health);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_HealthMax);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Mana);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_ManaMax);
	
	/** Camera Tags */
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_Locked);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_FollowingHero);
	
	/** Camera Ability Tags */
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ShowDraft);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ShowGameplayHUD);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ShowOverhead);
	MYDOTA_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Camera_FollowingHero);
}
