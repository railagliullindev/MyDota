// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystem/Abilities/General/GA_HeroDeath.h"

#include "AbilitySystemComponent.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Characters/MD_CharacterBase.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "GameModes/MD_GameMode.h"

UGA_HeroDeath::UGA_HeroDeath()
{
	// 1. Создаем один объект на героя и гоняем его по кругу
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 2. Только сервер решает, когда запускать смерть
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// 3. Клиент не может сам "умереть" или "воскреснуть" через читы
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyExecution;

	// 4. Добавляем триггер, чтобы HandleGameplayEvent ее "будил"
	FAbilityTriggerData DeathTrigger;
	DeathTrigger.TriggerTag = MyDotaTags::Status_Death;
	DeathTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(DeathTrigger);
}

void UGA_HeroDeath::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FString AutorityString = HasAuthority(&ActivationInfo) ? TEXT("Server") : TEXT("Client");
	UE_LOG(LogTemp, Warning, TEXT("[%s] UGA_HeroDeath::ActivateAbility Start timer"), *AutorityString);

	// 1. Прерываем все текущие абилки героя (Channeling, Movement и т.д.)
	// Кроме тех, что имеют тег Ability.CanBeUsedWhileDead
	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(MyDotaTags::Ability);
	GetAbilitySystemComponentFromActorInfo()->CancelAbilities(&CancelTags);

	// 2. Только на сервере считаем время смерти и запускаем логику
	if (HasAuthority(&CurrentActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server] UGA_HeroDeath::ActivateAbility Start timer"));

		// В Dota время респавна зависит от уровня: Level * 2 + 5 (условно)
		float RespawnTime = 5.f; //  Здесь логика расчета времени (Level * 2 + 5)

		float FinishTime = GetWorld()->GetTimeSeconds() + RespawnTime;

		if (AMD_PlayerState* PS = Cast<AMD_PlayerState>(GetOwningActorFromActorInfo()))
		{
			PS->RespawnTimeFinished = FinishTime;
			PS->OnRep_RespawnTimeFinished(); // Форсируем вызов для сервера
		}

		// Серверный таймер на фактическое воскрешение
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &UGA_HeroDeath::OnRespawnFinished, RespawnTime, false);
	}

	// На клиенте здесь можно включить пост-процесс "черно-белый экран"
}

void UGA_HeroDeath::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

	const FString AutorityString = HasAuthority(&ActivationInfo) ? TEXT("Server") : TEXT("Client");

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	bool HasTag = ASC->HasMatchingGameplayTag(MyDotaTags::Status_Death);

	UE_LOG(LogTemp, Warning, TEXT("[%s] UGA_HeroDeath::EndAbility HasTag %s"), *AutorityString, HasTag ? TEXT("TRUE") : TEXT("FALSE"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HeroDeath::OnRespawnFinished()
{
	if (AMD_PlayerState* PS = Cast<AMD_PlayerState>(GetOwningActorFromActorInfo()))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_HeroDeath::OnRespawnFinished RESET RESPAWN TIME"));
		PS->RespawnTimeFinished = 0.0f; // Сбрасываем таймер
		PS->OnRep_RespawnTimeFinished();
	}

	if (!HasAuthority(&CurrentActivationInfo)) return;

	UE_LOG(LogTemp, Warning, TEXT("[Server] UGA_HeroDeath::OnRespawnFinished"));

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	AMD_PlayerState* PS = Cast<AMD_PlayerState>(GetOwningActorFromActorInfo());
	AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(GetAvatarActorFromActorInfo());
	AMD_GameMode* GM = GetWorld()->GetAuthGameMode<AMD_GameMode>();

	if (ASC && Hero)
	{
		// 1. Удаляем эффект смерти (это автоматически снимет тег Status.Death на всех клиентах)
		ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(MyDotaTags::Status_Death));

		// 2. Восстанавливаем статы (мгновенно)
		const UMD_AttributeSet* AS = Cast<UMD_AttributeSet>(ASC->GetAttributeSet(UMD_AttributeSet::StaticClass()));
		ASC->SetNumericAttributeBase(AS->GetHealthAttribute(), AS->GetHealthMax());

		// 3. Телепортация и визуал
		FVector SpawnLoc = GM->GetBaseLocation(PS->GetTeam());
		Hero->OnRespawnAction(SpawnLoc);

		// 4. GameplayCue на появление
		ASC->ExecuteGameplayCue(MyDotaTags::GameplayCue_Character_Respawn);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}