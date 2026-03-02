// Rail Agliullin Dev. All Rights Reserved


#include "Actors/Projectile/MD_ProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


AMD_ProjectileBase::AMD_ProjectileBase()
{	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	
	ProjectileMovement->ProjectileGravityScale = 0.f; // Летим по прямой без падения
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bInitialVelocityInLocalSpace = true;
	
	// --- НАСТРОЙКИ ГОМИНГА (Dota-style) ---
	ProjectileMovement->bIsHomingProjectile = true;
	// Насколько резко снаряд будет доворачивать (чем выше, тем меньше шансов увернуться)
	ProjectileMovement->HomingAccelerationMagnitude = 5000.f;
	
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
}


void AMD_ProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	Sphere->IgnoreActorWhenMoving(GetOwner(), true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMD_ProjectileBase::OnSphereOverlap);
}

void AMD_ProjectileBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() && DamageEffectSpecHandle.IsValid() && OtherActor)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
			Destroy();
		}
	}
}

