// Rail Agliullin Dev. All Rights Reserved


#include "Controllers/MD_PlayerController.h"

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Characters/MD_CharacterBase.h"
#include "Net/UnrealNetwork.h"

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


void AMD_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (ClickMoveMappingContext)
			{
				Subsystem->AddMappingContext(ClickMoveMappingContext, /*Priority*/0);
			}
		}
	}
	
	/*if (HasAuthority())
	{
		SpawnHero();
	}*/
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
		UE_LOG(LogTemp, Display, TEXT("InputMove"));
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
		else
		{
			// Если AIC нет, пробуем заставить его заспавниться
			Hero->SpawnDefaultController();
			UE_LOG(LogTemp, Warning, TEXT("AI Controller отсутствовал, пробуем заспавнить..."));
		}
	}
}

void AMD_PlayerController::SpawnHero()
{	
	// ИСПРАВЛЕНИЕ: Убираем Owner = this. 
	// Герой должен принадлежать миру или AI, а не напрямую игроку.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr; // <--- КРИТИЧНО для MOBA
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Спавним
	Hero = GetWorld()->SpawnActor<AMD_CharacterBase>(HeroClass, FVector(0,0,100), FRotator::ZeroRotator, SpawnParams);
    
	if (Hero)
	{
		// Ручной вызов, так как на сервере OnRep не стреляет сам
		OnRep_Hero(); 
	}
}
