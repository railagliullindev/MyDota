// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/Caracters/GA_RangedAttack.h"

#include "MD_GameplayTags.h"
#include "Actors/Projectile/MD_ProjectileBase.h"
#include "Characters/MD_CharacterBase.h"

UGA_RangedAttack::UGA_RangedAttack(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer{MyDotaTags::Ability_Attack});
}

void UGA_RangedAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	if (!MyTarget.IsValid() || !GetActorInfo().IsNetAuthority()) return;

	AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(GetAvatarActorFromActorInfo());
	
	if (Hero)
	{
		FVector SpawnLoc = GetAvatarActorFromActorInfo()->GetActorLocation() + FVector(0,0,50);
		FRotator SpawnRot = (MyTarget->GetActorLocation() - SpawnLoc).Rotation();
		
		// Готовим урон на сервере
		FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		
		Hero->Multicast_SpawnProjectile(ProjectileClass, SpawnLoc, SpawnRot, MyTarget.Get());
	}
}
