// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/Caracters/GA_RangedAttack.h"

#include "MD_GameplayTags.h"
#include "Actors/Projectile/MD_ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"

UGA_RangedAttack::UGA_RangedAttack(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer{MyDotaTags::Ability_Attack});
}

void UGA_RangedAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	if (!MyTarget.IsValid() || !GetActorInfo().IsNetAuthority()) return;

	// 1. Точка спавна (из руки героя)
	FVector SpawnLoc = GetAvatarActorFromActorInfo()->GetActorLocation() + FVector(0,0,50);
	FRotator SpawnRot = (MyTarget->GetActorLocation() - SpawnLoc).Rotation();

	// 2. Спавн
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetAvatarActorFromActorInfo();
	
	AMD_ProjectileBase* Projectile = GetWorld()->SpawnActor<AMD_ProjectileBase>(ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
    
	if (Projectile && Projectile->ProjectileMovement)
	{
		// 3. ПЕРЕДАЕМ ДАННЫЕ ДЛЯ НАВЕДЕНИЯ
		// В Dota 2 снаряд летит в центр коллизии (RootComponent)
		Projectile->ProjectileMovement->HomingTargetComponent = MyTarget->GetRootComponent();
        
		// Передаем урон
		Projectile->DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	}
}
