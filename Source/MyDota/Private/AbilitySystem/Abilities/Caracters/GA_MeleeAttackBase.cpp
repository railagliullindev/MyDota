// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/Caracters/GA_MeleeAttackBase.h"

#include "AIController.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Navigation/PathFollowingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_MeleeAttackBase::UGA_MeleeAttackBase()
{
	SetAssetTags(FGameplayTagContainer{MyDotaTags::Ability_Attack});
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UGA_MeleeAttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	MyTarget = const_cast<AActor*>(TriggerEventData->Target.Get());
	
	if (!MyTarget.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true); 
		return;
	}
	
	
	// 1. Достаем ASC из ActorInfo
	UAbilitySystemComponent* ASC = GetActorInfo().AbilitySystemComponent.Get();
    
	// 2. Достаем наш конкретный сет атрибутов
	const UMD_AttributeSet* AS = GetAttributeSet();
    
	if (AS)
	{
		// 3. Теперь у тебя есть доступ ко всем статам!
		AttackRange = AS->GetAttackRange();
		AttackSpeed = AS->GetAttackSpeed();
	}
	
	AIC = Cast<AAIController>(Cast<APawn>(GetAvatarActorFromActorInfo())->GetController());
	
	RunAttackLoop();
}

void UGA_MeleeAttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const UMD_AttributeSet* UGA_MeleeAttackBase::GetAttributeSet() const
{
	return GetAbilitySystemComponentFromActorInfo()->GetSet<UMD_AttributeSet>();
}

void UGA_MeleeAttackBase::RunAttackLoop()
{
	float Dist = FVector::Distance(GetAvatarActorFromActorInfo()->GetActorLocation(), MyTarget->GetActorLocation());
	
	if (Dist <= AttackRange)
	{
		AIC->StopMovement();
		
		PlayAttackMontage();
	}else
	{
		// Если далеко — командуем AI идти к цели (MoveToActor)
		MoveToTarget();
        
		// Ждем немного и проверяем снова (через задержку в абилке)
		GetWorld()->GetTimerManager().SetTimerForNextTick([this](){ RunAttackLoop();});
	}
}

void UGA_MeleeAttackBase::MoveToTarget()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !MyTarget.IsValid()) return;
	
	if (AIC)
	{
		FAIMoveRequest MoveRequest(MyTarget.Get());
		MoveRequest.SetAcceptanceRadius(AttackRange * 0.9f);
		MoveRequest.SetReachTestIncludesAgentRadius(true);
		
		AIC->MoveTo(MoveRequest);
	}
}

void UGA_MeleeAttackBase::PlayAttackMontage()
{
	if (!AttackMontage) return;

	// Рассчитываем скорость анимации на основе AttackSpeed (Dota-style)
	// Если AS = 1.0 (базовый), скорость 1.0. Если AS = 2.0 (ускорен), скорость 2.0.
	float PlayRate = AttackSpeed;

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, 
		NAME_None, 
		AttackMontage, 
		PlayRate
	);

	// Подписываемся на завершение монтажа, чтобы запустить следующий цикл атаки
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAttackBase::OnAttackMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAttackBase::OnAttackMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAttackBase::OnAttackMontageFinished);

	MontageTask->ReadyForActivation();
}

void UGA_MeleeAttackBase::OnAttackMontageFinished()
{
	RunAttackLoop();
}
