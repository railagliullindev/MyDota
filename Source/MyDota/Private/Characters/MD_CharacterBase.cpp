// Rail Agliullin Dev. All Rights Reserved


#include "Characters/MD_CharacterBase.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"

AMD_CharacterBase::AMD_CharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	GetMesh()->bReceivesDecals = false;
	
	MD_AbilitySystemComponent = CreateDefaultSubobject<UMD_AbilitySystemComponent>(TEXT("AbilitySystem"));
	MD_AttributeSet = CreateDefaultSubobject<UMD_AttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AMD_CharacterBase::GetAbilitySystemComponent() const
{
	return GetMDAbilitySystemComponent();
}
