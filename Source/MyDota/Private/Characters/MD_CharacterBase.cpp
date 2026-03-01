// Rail Agliullin Dev. All Rights Reserved


#include "Characters/MD_CharacterBase.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "MyDota/MyDota.h"
#include "Net/UnrealNetwork.h"

void AMD_CharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMD_CharacterBase, PS)
}


AMD_CharacterBase::AMD_CharacterBase()
{
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 320.f;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	MD_AttributeSet = CreateDefaultSubobject<UMD_AttributeSet>(TEXT("AttributeSet"));
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* AMD_CharacterBase::GetAbilitySystemComponent() const
{
	return MD_AbilitySystemComponent;
}

void AMD_CharacterBase::SetPlayerState(AMD_PlayerState* InPs)
{
	PS = InPs;
	
	InitAbilitySystem();
}

void AMD_CharacterBase::InitAbilitySystem()
{
	if (PS)
	{
		// Аналогично инициализируем на клиент
		MD_AbilitySystemComponent = Cast<UMD_AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		MD_AttributeSet = PS->GetAttributeSet();
		
		// ВАЖНО: Owner — PlayerState, Avatar — Character (тело)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		
		UE_LOG(LogMyDotaGAS, Log, TEXT("[%s] GAS инициализирован для %s через PlayerState"), 
		   HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetName());
		
		
		if (!HeroStartupData.IsNull())
		{
			if (UDataAsset_HeroStartupData* LoadedData = HeroStartupData.LoadSynchronous())
			{
				LoadedData->GiveToAbilitySystemComponent(MD_AbilitySystemComponent);
			}
		}
		
	}
}

// CLIENT only
void AMD_CharacterBase::OnRep_PlayerState()
{
	if (!PS) return;
	
	UE_LOG(LogMyDotaGAS, Log, TEXT("[%s] OnRep_PlayerState"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
	Super::OnRep_PlayerState();
	
	//InitAbilitySystem();
}

void AMD_CharacterBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	UE_LOG(LogMyDotaGAS, Log, TEXT("[%s] OnRep_Owner"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
	//InitAbilitySystem();
}

