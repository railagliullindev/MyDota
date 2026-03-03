// Rail Agliullin Dev. All Rights Reserved

#include "Characters/MD_CharacterBase.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Actors/Projectile/MD_ProjectileBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "MyDota/MyDota.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Overhead/MD_OverheadWidget.h"
#include "Subsystems/FogOfWarManager.h"

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
	bUseControllerRotationYaw = true;
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

EMDTeam AMD_CharacterBase::GetTeam() const
{
	UE_LOG(LogTemp, Warning, TEXT("GetTeam As PS valid? = %s"), PS ? TEXT("True") : TEXT("False"));
	return PS ? PS->GetTeam() : EMDTeam::None;
}

FGenericTeamId AMD_CharacterBase::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)GetTeam());
}

void AMD_CharacterBase::SetPlayerState(AMD_PlayerState* InPs)
{
	PS = InPs;

	InitAbilitySystem();

	// Проверяем, что мы на сервере (HasAuthority)
	// или это не клиентская копия в мультиплеере
	if (HasAuthority())
	{
		AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, (uint8)GetTeam());
		if (FogManager)
		{
			// Регистрируем юнит как источник обзора
			FogManager->RegisterSource(this, 1200.0f);
			UE_LOG(LogTemp, Warning, TEXT("Server: Registered %s as Vision Source"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Server: FogManager NOT FOUND during BeginPlay!"));
		}
	}
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

		UE_LOG(LogMyDotaGAS, Log, TEXT("[%s] GAS инициализирован для %s через PlayerState"), HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetName());

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

void AMD_CharacterBase::Multicast_SpawnProjectile_Implementation(TSubclassOf<AActor> ProjClass, FVector Loc, FRotator Rot, AActor* Target)
{
	// Этот код выполняется на СЕРВЕРЕ и на ВСЕХ КЛИЕНТАХ
	if (!ProjClass || !Target) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; // Владелец - наш герой
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMD_ProjectileBase* Projectile = GetWorld()->SpawnActor<AMD_ProjectileBase>(ProjClass, Loc, Rot, SpawnParams);

	if (Projectile && Projectile->ProjectileMovement)
	{
		// Привязываем цель для самонаведения (локально у каждого)
		Projectile->ProjectileMovement->HomingTargetComponent = Target->GetRootComponent();

		// Передаем данные об уроне (валидны только на сервере)
		if (HasAuthority())
		{
			auto Spec = MakeDamageSpec(DefaultAttackDamageEffect, 1);
			Projectile->DamageEffectSpecHandle = Spec; // Сохрани спеку в переменной героя перед мультикастом
		}
	}
}

FGameplayEffectSpecHandle AMD_CharacterBase::MakeDamageSpec(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	// 1. Проверяем наличие класса эффекта и ASC
	if (!EffectClass || !ASC)
	{
		return FGameplayEffectSpecHandle();
	}

	// 2. Создаем контекст (хранит информацию о том, КТО наносит урон)
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);
	// Context.AddInstigator(this, this);// AddByrefSourceActors(this, this); // Добавляем себя как источник

	// 3. ВЫЗЫВАЕМ МЕТОД У КОМПОНЕНТА (ASC)
	// Именно здесь создается "пакет данных" об уроне
	return ASC->MakeOutgoingSpec(EffectClass, Level, Context);
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
	// InitHealthBar();
}
