// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MD_CharacterBase.generated.h"

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
	
	void SetPlayerState(AMD_PlayerState* InPs);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName HeroName = "Axe";
	
protected:
	
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
	UMD_AbilitySystemComponent* MD_AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
	UMD_AttributeSet* MD_AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StartupData")
	TSoftObjectPtr<UDataAsset_HeroStartupData> HeroStartupData;
	
	UPROPERTY(ReplicatedUsing = OnRep_PlayerState)
	AMD_PlayerState* PS;
	
	virtual void InitAbilitySystem();
	
	// Переопределения для мультиплеера
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Owner() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
