// Rail Agliullin Dev. All Rights Reserved

#include "Characters/MD_CharacterBase.h"

#include "AIController.h"
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
#include "Pawns/MD_CameraPawn.h"
#include "Systems/FogOfWar/FogOfWarManager.h"
#include "Widgets/World/WorldOverheadWidget.h"

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

void AMD_CharacterBase::HandleDeath()
{
	// 1. Капсула больше не блокирует мир (чтобы враги могли проходить "сквозь" труп)
	// Но мы оставляем её живой, чтобы актор не удалился
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 2. Настраиваем Меш для Ragdoll
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		// Включаем коллизию меша, чтобы он видел пол
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// Убеждаемся, что меш блокирует WorldStatic (пол)
		MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		// Включаем симуляцию
		MeshComp->SetSimulatePhysics(true);

		// Опционально: даем импульс в сторону урона, чтобы тело "отлетело"
		// MeshComp->AddImpulse(DeathImpulse);
	}

	// 3. Останавливаем движение (чтобы труп не скользил по инерции)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
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

void AMD_CharacterBase::OnRespawnAction(FVector RespawnLocation)
{
	// 1. Останавливаем все текущие движения и инерцию
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement(); // Временно отключаем, пока телепортируем

		// Сбрасываем флаги падения/плавания, чтобы герой стоял ровно
		MoveComp->SetDefaultMovementMode();
	}

	// 2. Телепортация
	// Используем ETeleportType::TeleportPhysics, чтобы не было "растягивания" физических костей
	SetActorLocation(RespawnLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// 3.Capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionProfileName(EName::Pawn); // SetCollisionResponseToAllChannels(ECR_Ignore);

	// 4. Возвращаем меш из Ragdoll (если он был)
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(false);

		// Возвращаем коллизию персонажа (для получения урона/кликов)
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Block); // Или ваши настройки

		// Важно: При Ragdoll меш отрывается от капсулы. Возвращаем его на место.
		MeshComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// Сбрасываем локальные трансформации меша к исходным (из Blueprints/Constructor)
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

		// Обнуляем скорость костей
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	// 5. Включаем движение обратно
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	// 6. Визуальное обновление (только если скрывали актора)
	SetActorHiddenInGame(false);

	// Если используете AIController для кликов (как в Dota), сбросьте его путь
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
	}

	if (IsLocallyControlled())
	{
		// Находим наш CameraPawn через PlayerController
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AMD_CameraPawn* CamPawn = Cast<AMD_CameraPawn>(PC->GetPawn()))
			{
				// Устанавливаем положение камеры над героем
				CamPawn->SetActorLocation(RespawnLocation); // SnapToLocation(RespawnLocation);
			}
		}
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
		if (AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, static_cast<uint8>(GetTeam())))
		{
			// Регистрируем юнит как источник обзора
			FogManager->UnRegisterSource(this);
			UE_LOG(LogTemp, Log, TEXT("Server: Unregistered %s as Vision Source"), *GetName());
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
	return FGenericTeamId(static_cast<uint8>(GetTeam()));
}

bool AMD_CharacterBase::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	const IFogOfWarTeamInterface* ViewerTeam = Cast<const IFogOfWarTeamInterface>(RealViewer);
	AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, static_cast<uint8>(ViewerTeam->GetTeam()));
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
		if (AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, static_cast<uint8>(GetTeam())))
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

		PS->GetAttributeSet()->OnHeroDied.AddDynamic(this, &AMD_CharacterBase::HandleDeath);

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
	// InitAbilitySystem();
}
