// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/Attacks/GA_AttackBase.h"

#include "AIController.h"
#include "MD_GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"

UGA_AttackBase::UGA_AttackBase(const FObjectInitializer& Initializer) : Super(Initializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	AcceptanceAngle = 10.f;
}

void UGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	
	RunAttackLoop();
}

void UGA_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	MyTarget = nullptr;
	
	if (AIC)
	{
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const UMD_AttributeSet* UGA_AttackBase::GetAttributeSet() const
{
	return GetAbilitySystemComponentFromActorInfo()->GetSet<UMD_AttributeSet>();
}

void UGA_AttackBase::RunAttackLoop()
{
	if (!MyTarget.IsValid()) { K2_EndAbility(); }
	
	if (GetDistanceToTarget() <= AttackRange)
	{
		if (AIC)
		{
			AIC->StopMovement();
		}
		
		if (IsFacing())
		{
			PlayAttackMontage();
		}else
		{
			if (AIC)
			{
				UE_LOG(LogTemp, Warning, TEXT("@@ Set FOCUS"));
				AIC->SetFocus(MyTarget.Get(), EAIFocusPriority::Gameplay);
			}
			// Ждем немного и проверяем снова (через задержку в абилке)
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::RunAttackLoop, 0.1f);
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
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::RunAttackLoop, 0.1f);
	}
}

void UGA_AttackBase::MoveToTarget()
{
	if (!MyTarget.IsValid()) return;
	
	if (AIC)
	{
		FAIMoveRequest MoveRequest(MyTarget.Get());
		MoveRequest.SetAcceptanceRadius(AttackRange * 0.5f);
		MoveRequest.SetReachTestIncludesAgentRadius(true);
		
	
		AIC->MoveTo(MoveRequest);
	}
}

void UGA_AttackBase::PlayAttackMontage()
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
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
	WaitEventTask->ReadyForActivation();
	
	// --- ЗАДАЧА 2: Проигрываем анимацию (как делали раньше)
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, 
		NAME_None, 
		AttackMontage, 
		PlayRate
	);

	// Подписываемся на завершение монтажа, чтобы запустить следующий цикл атаки
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnAttackMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnAttackMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnAttackMontageFinished);

	MontageTask->ReadyForActivation();
}

float UGA_AttackBase::GetDistanceToTarget() const
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

bool UGA_AttackBase::IsFacing() const
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

void UGA_AttackBase::OnHitEventReceived(FGameplayEventData Payload)
{
}

void UGA_AttackBase::OnAttackMontageFinished()
{
	if (MyTarget.IsValid())
	{
		RunAttackLoop();
	}else
	{
		K2_EndAbility();
	}
}

FGameplayAbilityTargetDataHandle UGA_AttackBase::GetAbilityTargetData()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
	NewData->TargetActorArray.Add(MyTarget);
	TargetDataHandle.Add(NewData);
	return TargetDataHandle;
}
