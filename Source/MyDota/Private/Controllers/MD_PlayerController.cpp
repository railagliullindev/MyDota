// Rail Agliullin Dev. All Rights Reserved


#include "Controllers/MD_PlayerController.h"

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Characters/MD_CharacterBase.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "GameModes/MD_GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Pawns/MD_CameraPawn.h"

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

void AMD_PlayerController::SelectHero(TSubclassOf<AMD_CharacterBase> InHeroClass)
{
	if (AMD_PlayerState* PS = GetPlayerState<AMD_PlayerState>())
	{
		PS->Server_SetSelectedHero(InHeroClass);
	}
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
	if (IsLocalController())
	{
		FString RoleString = HasAuthority() ? TEXT("ListenServer-Host") : TEXT("Remote-Client");
        
		if (Hero)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Герой успешно привязан к контроллеру: %s"), 
				*RoleString, *Hero->GetName());
                
			// ТУТ инициализируй HUD или камеру
		}
	}
}

void AMD_PlayerController::SetMatchMode_Implementation(EMathStage InMatchStage)
{
	MatchStage = InMatchStage;
	
	switch (MatchStage)
	{
	case EMathStage::Draft:
		OnDraftMode();
		break;
	case EMathStage::PreGame:
		break;
	case EMathStage::InProgress:
		OnMatchMode();
		break;
	case EMathStage::PostGame:
		break;
	}
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

void AMD_PlayerController::Server_MoveToLocation_Implementation(FVector InLocation)
{
	if (Hero)
	{
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
			UE_LOG(LogTemp, Display, TEXT("AI двигает героя в %s"), *InLocation.ToString());
			AIC->MoveToLocation(InLocation, 5.f, true, true, true);
		}
	}
}

void AMD_PlayerController::SpawnHero()
{	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr; // <--- КРИТИЧНО для MOBA
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	Hero = GetWorld()->SpawnActor<AMD_CharacterBase>(HeroClass, FVector(0,0,100), FRotator::ZeroRotator, SpawnParams);
    
	if (Hero)
	{
		OnRep_Hero(); 
	}
}

void AMD_PlayerController::OnDraftMode()
{
	if (IsLocalController() && DraftWidgetClass)
	{
		DraftWidget = CreateWidget<UUserWidget>(this, DraftWidgetClass);
		if (DraftWidget)
		{            
			DraftWidget->AddToViewport(0);
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(DraftWidget->GetCachedWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
	}
}

void AMD_PlayerController::OnMatchMode()
{
	if (DraftWidget)
	{
		DraftWidget->RemoveFromParent();
		DraftWidget = nullptr;
		
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (ClickMoveMappingContext)
			{
				Subsystem->AddMappingContext(ClickMoveMappingContext,0);
			}
		}
	}
}
