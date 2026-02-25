// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MD_CharacterBase.generated.h"

class UMD_AttributeSet;
class UMD_AbilitySystemComponent;

UCLASS()
class MYDOTA_API AMD_CharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMD_CharacterBase();
	
	//~ Begin IAbilitySystemInterface Interface.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface Interface
	
	FORCEINLINE UMD_AbilitySystemComponent* GetMDAbilitySystemComponent() const {return MD_AbilitySystemComponent;}
	FORCEINLINE UMD_AttributeSet* GetMDAttributeSet() const {return MD_AttributeSet;}

	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UMD_AbilitySystemComponent* MD_AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UMD_AttributeSet* MD_AttributeSet;
};
