// Rail Agliullin Dev. All Rights Reserved

#include "Characters/MD_CharacterBase.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Actors/Projectile/MD_ProjectileBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Controllers/MD_PlayerController.h"
#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFrameworks/MD_GameState.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "MyDota/MyDota.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Overhead/MD_OverheadWidget.h"
#include "Subsystems/FogOfWarManager.h"

AMD_CharacterBase::AMD_CharacterBase()
{
	bReplicates = true;
	bAlwaysRelevant = false;
	ACharacter::SetReplicateMovement(true);
	SetNetCullDistanceSquared(400000000.f);

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

void AMD_CharacterBase::RegisterUnit()
{
	if (AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>())
	{
		GS->RegisterUnit(this);
	}
}

void AMD_CharacterBase::UnRegisterUnit()
{
	if (AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>())
	{
		GS->UnregisterUnit(this);
	}
}

bool AMD_CharacterBase::IsOwnedByLocalPlayer(AActor* InActor) const
{
	if (!InActor) return false;

	AController* MyPC = GetWorld()->GetFirstPlayerController();
	if (InActor->GetOwner())
	{
		return MyPC && InActor->GetOwner() == MyPC;
	}
	else
	{
		return false;
	}
}

UAbilitySystemComponent* AMD_CharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void AMD_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	RegisterUnit();

	InitHealthBar();
}

void AMD_CharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnRegisterUnit();

	if (HasAuthority())
	{
		if (AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, (uint8)GetTeam()))
		{
			// Регистрируем юнит как источник обзора
			FogManager->UnRegisterSource(this);
			UE_LOG(LogTemp, Warning, TEXT("Server: Unregistered %s as Vision Source"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Server: FogManager NOT FOUND during ~AMD_CharacterBase!"));
		}
	}

	Super::EndPlay(EndPlayReason);
}

EMDTeam AMD_CharacterBase::GetTeam() const
{
	return PS ? PS->GetTeam() : EMDTeam::None;
}

FGenericTeamId AMD_CharacterBase::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)GetTeam());
}

bool AMD_CharacterBase::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	const IMDTeamInterface* ViewerTeam = Cast<const IMDTeamInterface>(RealViewer);
	AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, (uint8)ViewerTeam->GetTeam());
	if (FogManager)
	{
		const AMD_PlayerController* PC = Cast<AMD_PlayerController>(RealViewer);

		if (PC && PC->GetHero())
		{
			const FIntPoint GridPos = FogManager->WorldToGrid(this->GetActorLocation()); // PC->GetHero()->GetActorLocation());
			return FogManager->IsCellVisible(GridPos);
		}
	}

	return false;
}

void AMD_CharacterBase::SetPlayerState(AMD_PlayerState* InPs)
{
	PS = InPs;

	InitAbilitySystem();

	// Проверяем, что мы на сервере (HasAuthority)
	// или это не клиентская копия в мультиплеере
	if (HasAuthority())
	{
		if (AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, (uint8)GetTeam()))
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
	UE_LOG(LogTemp, Warning, TEXT("[ %s ] OnRep_PlayerState"), *GetName());

	if (!PS) return;
	Super::OnRep_PlayerState();

	InitAbilitySystem();
}

void AMD_CharacterBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	InitAbilitySystem();
}
