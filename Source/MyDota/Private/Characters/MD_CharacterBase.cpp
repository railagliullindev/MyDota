// Rail Agliullin Dev. All Rights Reserved


#include "Characters/MD_CharacterBase.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "MyDota/MyDota.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Overhead/MD_OverheadWidget.h"

void AMD_CharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMD_CharacterBase, PS)
}

void AMD_CharacterBase::OnValueChanged(const FOnAttributeChangeData& Data)
{
	if (OverheadWidget)
	{
		OverheadWidget->UpdateStats(AS->GetHealth(), AS->GetHealthMax(), AS->GetMana(), AS->GetManaMax());
	}
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
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = 320.f;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetWidgetClass(OverheadWidgetClass);
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawAtDesiredSize(true);
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* AMD_CharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void AMD_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitHealthBar();
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
		ASC = Cast<UMD_AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AS = PS->GetAttributeSet();
		
		// ВАЖНО: Owner — PlayerState, Avatar — Character (тело)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
		
		UE_LOG(LogMyDotaGAS, Log, TEXT("[%s] GAS инициализирован для %s через PlayerState"), 
		   HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetName());
		
		
		if (!HasAuthority()) return;
		if (!HeroStartupData.IsNull())
		{
			if (UDataAsset_HeroStartupData* LoadedData = HeroStartupData.LoadSynchronous())
			{
				LoadedData->GiveToAbilitySystemComponent(ASC);
			}
		}
		
	}
}


void AMD_CharacterBase::InitHealthBar()
{		
	if (IsLocallyControlled() || GetNetMode() != NM_DedicatedServer)
	{
		if (ASC && AS)
		{
			OverheadWidget = Cast<UMD_OverheadWidget>(HealthBarComponent->GetUserWidgetObject());
			ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddUObject(this, &AMD_CharacterBase::OnValueChanged);
			ASC->GetGameplayAttributeValueChangeDelegate(AS->GetManaAttribute()).AddUObject(this, &AMD_CharacterBase::OnValueChanged);
		}
	}
}

// CLIENT only
void AMD_CharacterBase::OnRep_PlayerState()
{
	if (!PS) return;
	Super::OnRep_PlayerState();
	
	InitAbilitySystem();
}

void AMD_CharacterBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	InitAbilitySystem();
	//InitHealthBar();
}

