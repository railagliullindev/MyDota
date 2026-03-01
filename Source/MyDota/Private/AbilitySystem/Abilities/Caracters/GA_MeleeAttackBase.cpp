// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/Caracters/GA_MeleeAttackBase.h"

#include "AIController.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Navigation/PathFollowingComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "MyDota/MyDota.h"

UGA_MeleeAttackBase::UGA_MeleeAttackBase()
{
	SetAssetTags(FGameplayTagContainer{MyDotaTags::Ability_Attack});
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	AcceptanceAngle = 10.f;
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
	
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		AIC = Cast<AAIController>(Character->GetController());
	}
	
	//AIC = Cast<AAIController>(Cast<APawn>(GetAvatarActorFromActorInfo())->GetController());
	
	RunAttackLoop();
}

void UGA_MeleeAttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	MyTarget = nullptr;
	
	if (AIC)
	{
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const UMD_AttributeSet* UGA_MeleeAttackBase::GetAttributeSet() const
{
	return GetAbilitySystemComponentFromActorInfo()->GetSet<UMD_AttributeSet>();
}

void UGA_MeleeAttackBase::RunAttackLoop()
{
	if (!MyTarget.IsValid()) { K2_EndAbility(); }
	
	if (GetDistanceToTarget() <= AttackRange)
	{
		if (AIC)
		{
			AIC->StopMovement();
			AIC->SetFocus(MyTarget.Get(), EAIFocusPriority::Gameplay);
		}
		
		if (IsFacing())
		{
			PlayAttackMontage();
		}else
		{
			// Ждем немного и проверяем снова (через задержку в абилке)
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGA_MeleeAttackBase::RunAttackLoop, 0.1f);
		}
		
	}else
	{
		// Если далеко — командуем AI идти к цели (MoveToActor)
		MoveToTarget();
		
		if (AIC)
		{
			AIC->ClearFocus(EAIFocusPriority::Gameplay);
		}
		
		// Ждем немного и проверяем снова (через задержку в абилке)
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UGA_MeleeAttackBase::RunAttackLoop, 0.1f);
	}
}

void UGA_MeleeAttackBase::MoveToTarget()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !MyTarget.IsValid()) return;
	
	if (AIC)
	{
		FAIMoveRequest MoveRequest(MyTarget.Get());
		MoveRequest.SetAcceptanceRadius(AttackRange * 0.5f);
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

	// --- ЗАДАЧА 1: Ждем событие удара ---
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		MyDotaTags::Event_Montage_Hit, 
		nullptr, 
		true, 
		false
	);
	
	// Подписываемся на получение события
	WaitEventTask->EventReceived.AddDynamic(this, &UGA_MeleeAttackBase::OnHitEventReceived);
	WaitEventTask->ReadyForActivation();
	
	// --- ЗАДАЧА 2: Проигрываем анимацию (как делали раньше)
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

float UGA_MeleeAttackBase::GetDistanceToTarget() const
{
	const float Distance = FVector::DistXY(GetAvatarActorFromActorInfo()->GetActorLocation(), MyTarget->GetActorLocation());

	// 2. Вычитаем радиусы капсул (коллизий)
	float CombinedRadius = 0.f;
    
	if (ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		CombinedRadius += AvatarChar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}
    
	if (ACharacter* TargetChar = Cast<ACharacter>(MyTarget.Get()))
	{
		CombinedRadius += TargetChar->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}

	// Итоговое расстояние "от края до края"
	return FMath::Max(0.f, Distance - CombinedRadius);
}

bool UGA_MeleeAttackBase::IsFacing() const
{
	if (!MyTarget.IsValid()) return false;
	
	FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	FVector DirectionToTarget = (MyTarget->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).GetSafeNormal();
    
	// Убираем вертикальную составляющую
	Forward.Z = 0.f;
	DirectionToTarget.Z = 0.f;
	
	const float DotProduct = FVector::DotProduct(Forward, DirectionToTarget);
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
	
	return Angle <= AcceptanceAngle;
}

void UGA_MeleeAttackBase::OnHitEventReceived(FGameplayEventData Payload)
{
	if (!MyTarget.IsValid()) return;
	
	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	
	if (DamageSpec.IsValid())
	{
		// 2. Накладываем эффект на цель (MyTarget)
		// Используем ApplyGameplayEffectSpecToTarget для мультиплеерной надежности
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageSpec, GetAbilityTargetData());
	}
}

void UGA_MeleeAttackBase::OnAttackMontageFinished()
{
	if (MyTarget.IsValid())
	{
		RunAttackLoop();
	}
}

FGameplayAbilityTargetDataHandle UGA_MeleeAttackBase::GetAbilityTargetData()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
	NewData->TargetActorArray.Add(MyTarget);
	TargetDataHandle.Add(NewData);
	return TargetDataHandle;
}
