// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/MyDotaGameplayAbility.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MD_AnimNotify_GameplayEvent.generated.h"
/**
 * 
 */
UCLASS()
class MYDOTA_API UMD_AnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, Category = "GAS", meta = (Categories = "Event"))
	FGameplayTag EventTag;
	
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayEventData EventData;
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
