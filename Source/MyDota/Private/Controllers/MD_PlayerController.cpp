// Rail Agliullin Dev. All Rights Reserved

#include "Controllers/MD_PlayerController.h"

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "Characters/MD_CharacterBase.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "GameModes/MD_GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Systems/FogOfWar/FogOfWarManager.h"

AMD_PlayerController::AMD_PlayerController()
{
	GenericTeamId = FGenericTeamId(0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

FGenericTeamId AMD_PlayerController::GetGenericTeamId() const
{
	return GenericTeamId;
}

void AMD_PlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMD_PlayerController, Hero);
}

EMDTeam AMD_PlayerController::GetTeam() const
{
	if (AMD_PlayerState* PS = GetPlayerState<AMD_PlayerState>())
	{
		return PS->GetTeam();
	}
	return EMDTeam::None;
}

void AMD_PlayerController::SelectHero_Implementation(const int32 InHeroID)
{
	AMD_GameMode* GM = GetWorld()->GetAuthGameMode<AMD_GameMode>();
	if (!GM) return;

	GM->ProcessHeroSelection(this, InHeroID);
}

void AMD_PlayerController::SetHero(AMD_CharacterBase* InHero)
{
	if (HasAuthority())
	{
		Hero = InHero;

		OnRep_Hero();
	}
}

void AMD_PlayerController::OnRep_Hero()
{
}

void AMD_PlayerController::SetMatchMode_Implementation(EMathStage InMatchStage)
{
	MatchStage = InMatchStage;

	switch (MatchStage)
	{
		case EMathStage::Draft: break;
		case EMathStage::PreGame: break;
		case EMathStage::InProgress:
			if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					if (ClickMoveMappingContext)
					{
						Subsystem->AddMappingContext(ClickMoveMappingContext, 0);
					}
				}

				AFogOfWarManager* FogOfWarManager = AFogOfWarManager::Get(GetWorld(), (uint8)GetTeam());
				if (FogOfWarManager)
				{
					FogOfWarManager->OnUnitVisibilityChanged.AddUObject(this, &AMD_PlayerController::OnUnitVisibilityChanged);
				}
			}
			break;
		case EMathStage::PostGame: break;
		default: break;
	}
}

void AMD_PlayerController::OnUnitVisibilityChanged(AActor* Actor, bool bArg)
{
	// Только вражеские Actor'ы, своих мы всегда видим
	Actor->SetActorHiddenInGame(!bArg);
}

void AMD_PlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMD_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickMoveAction)
		{
			EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &AMD_PlayerController::InputMove);
		}
		if (AttackAction)
		{
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AMD_PlayerController::InputAttack);
		}
	}
}

void AMD_PlayerController::InputMove()
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		Server_MoveToLocation(Hit.ImpactPoint);

		// Визуал клика
	}
}

void AMD_PlayerController::InputAttack()
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Pawn, false, Hit))
	{
		AActor* Target = Hit.GetActor();
		if (Target)
		{
			if (Target == Hero) return;
			if (!Cast<APawn>(Hit.GetActor())) return;

			Server_AttackTarget(Target);
		}
	}
}

void AMD_PlayerController::Server_MoveToLocation_Implementation(FVector InLocation)
{

	if (Hero)
	{
		if (UMD_AbilitySystemComponent* ASC = Cast<UMD_AbilitySystemComponent>(Hero->GetAbilitySystemComponent()))
		{
			ASC->CancelAbilityWithTag(MyDotaTags::Ability_Attack);
		}

		// ИСПРАВЛЕНИЕ ДЛЯ ХОСТА:
		// Проверяем, не завладел ли случайно PlayerController этим героем
		if (Hero->GetController() == this)
		{
			UE_LOG(LogTemp, Error, TEXT("Хост владеет героем напрямую! AI отключен."));
			// Если это случилось, нужно сделать UnPossess и дать AI перехватить управление
			UnPossess();
		}

		AAIController* AIC = Cast<AAIController>(Hero->GetController());
		if (AIC)
		{
			AIC->MoveToLocation(InLocation, 5.f, true, true, true);
		}
	}
}

void AMD_PlayerController::Server_AttackTarget_Implementation(AActor* Target)
{
	if (Hero && Target)
	{
		FGameplayEventData Payload;
		Payload.Target = Target;

		Hero->GetAbilitySystemComponent()->HandleGameplayEvent(MyDotaTags::Event_Ability_RequestAttack, &Payload);
	}
}

void AMD_PlayerController::SpawnHero()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr; // <--- КРИТИЧНО для MOBA
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	Hero = GetWorld()->SpawnActor<AMD_CharacterBase>(HeroClass, FVector(0, 0, 100), FRotator::ZeroRotator, SpawnParams);

	if (Hero)
	{
		OnRep_Hero();
	}
}
