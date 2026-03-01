// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MD_CharacterBase.generated.h"

struct FOnAttributeChangeData;
class UMD_OverheadWidget;
class UWidgetComponent;
class AMD_PlayerState;
class UDataAsset_HeroStartupData;
class UMD_AttributeSet;
class UMD_AbilitySystemComponent;

UCLASS()
class MYDOTA_API AMD_CharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMD_CharacterBase();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void BeginPlay() override;
	
	void SetPlayerState(AMD_PlayerState* InPs);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName HeroName = "Axe";
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* HealthBarComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMD_OverheadWidget> OverheadWidgetClass;
	
protected:
	
	// Переопределения для мультиплеера
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Owner() override;
	
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
	UMD_AbilitySystemComponent* ASC;
	
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
	UMD_AttributeSet* AS;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDataAsset_HeroStartupData> HeroStartupData;
	
	UPROPERTY(ReplicatedUsing = OnRep_PlayerState)
	AMD_PlayerState* PS;
	
	virtual void InitAbilitySystem();
	void InitHealthBar();
	
	UPROPERTY()
	UMD_OverheadWidget* OverheadWidget;
	

	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void OnValueChanged(const FOnAttributeChangeData& Data);
};
