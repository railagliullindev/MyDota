// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/MD_AnimNotify_GameplayEvent.h"

void UMD_AnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* Owner = MeshComp->GetOwner();
	
	if (Owner && Owner->HasAuthority())
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, EventData);
	}
}
