// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "Interfaces/MDTeamInterface.h"
#include "MD_CharacterBase.generated.h"

class UGameplayEffect;
struct FGameplayEffectSpecHandle;
struct FOnAttributeChangeData;
class UMD_OverheadWidget;
class UWidgetComponent;
class AMD_PlayerState;
class UDataAsset_HeroStartupData;
class UMD_AttributeSet;
class UMD_AbilitySystemComponent;

UCLASS()
class MYDOTA_API AMD_CharacterBase : public ACharacter, public IAbilitySystemInterface, public IMDTeamInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	AMD_CharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void BeginPlay() override;

	virtual EMDTeam GetTeam() const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	void SetPlayerState(AMD_PlayerState* InPs);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName HeroName = "Axe";

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* HealthBarComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMD_OverheadWidget> OverheadWidgetClass;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnProjectile(TSubclassOf<AActor> ProjClass, FVector Loc, FRotator Rot, AActor* Target);

	// Функция-фабрика: создает спеку на сервере
	FGameplayEffectSpecHandle MakeDamageSpec(TSubclassOf<UGameplayEffect> EffectClass, float Level);

protected:

	// Переопределения для мультиплеера
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Owner() override;

	virtual void InitAbilitySystem();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
	UMD_AbilitySystemComponent* ASC;

	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
	UMD_AttributeSet* AS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDataAsset_HeroStartupData> HeroStartupData;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerState)
	AMD_PlayerState* PS;

	UPROPERTY()
	UMD_OverheadWidget* OverheadWidget;

	// Переменная класса урона, которую можно настроить в BP героя
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	TSubclassOf<UGameplayEffect> DefaultAttackDamageEffect;

	// Временный кэш для передачи данных серверному снаряду.
	// НЕ реплицируем (UPROPERTY без параметров), чтобы не грузить сеть.
	UPROPERTY()
	FGameplayEffectSpecHandle CurrentProjectileSpec;

	void InitHealthBar();
	void OnValueChanged(const FOnAttributeChangeData& Data);
};
