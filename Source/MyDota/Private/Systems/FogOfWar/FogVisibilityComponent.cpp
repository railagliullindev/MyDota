// Rail Agliullin Dev. All Rights Reserved

#include "Systems/FogOfWar/FogVisibilityComponent.h"

#include "Characters/MD_CharacterBase.h"
#include "GameFramework/GameSession.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Interfaces/MDTeamInterface.h"
#include "Subsystems/FogOfWarManager.h"

UFogVisibilityComponent::UFogVisibilityComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
}

void UFogVisibilityComponent::BeginPlay()
{
	Super::BeginPlay();

	IsServer = GetOwner()->HasAuthority();
	AController* MyPC = GetWorld()->GetFirstPlayerController();
	AMD_PlayerState* PS = MyPC->GetPlayerState<AMD_PlayerState>();
	FString PSName;
	if (PS)
	{
		PSName = PS->GetName();
	}
	IsOwnedByLocalPlayer = PS == Cast<AMD_PlayerState>(GetOwner()) ? true : false;

	UE_LOG(LogTemp, Log, TEXT("Owner - %s, PS - %s"), *GetOwner()->GetName(), *PSName);

	UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::BeginPlay"), *GetOwnerString(), *GetOwner()->GetName());

	OnInit();
}

void UFogVisibilityComponent::OnInit()
{
	const IMDTeamInterface* ViewerTeam = Cast<const IMDTeamInterface>(GetOwner());
	if (IsServer)
	{
		FogManager = AFogOfWarManager::Get(this, (uint8)ViewerTeam->GetTeam());
		if (FogManager)
		{
			FogManager->OnUnitVisibilityChanged.AddUObject(this, &UFogVisibilityComponent::OnVisibilityChanged);
			UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit Success"), *GetOwnerString(), *GetOwner()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit FogManager is NULL"), *GetOwnerString(), *GetOwner()->GetName());
		}
	}
	else
	{
		if (IsOwnedByLocalPlayer)
		{
			FogManager = AFogOfWarManager::Get(this, (uint8)ViewerTeam->GetTeam());
			if (FogManager)
			{
				FogManager->OnUnitVisibilityChanged.AddUObject(this, &UFogVisibilityComponent::OnVisibilityChanged);
				UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit Success"), *GetOwnerString(), *GetOwner()->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit FogManager is NULL"), *GetOwnerString(), *GetOwner()->GetName());
			}
		}
		else
		{

			// FogManager = AFogOfWarManager::Get(this, (uint8)ViewerTeam->GetTeam());
			if (FogManager)
			{
				FogManager->OnUnitVisibilityChanged.AddUObject(this, &UFogVisibilityComponent::OnVisibilityChanged);
				UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit Success"), *GetOwnerString(), *GetOwner()->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit FogManager is NULL"), *GetOwnerString(), *GetOwner()->GetName());
			}
		}
	}

	/*const IMDTeamInterface* ViewerTeam = Cast<const IMDTeamInterface>(GetOwner());
	UE_LOG(LogTemp, Log, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit() is valid ViewerTeam %s"), *GetOwnerString(), *GetOwner()->GetName(), ViewerTeam ? TEXT("true") : TEXT("false"));
	if (ViewerTeam)
	{
		UE_LOG(LogTemp, Log, TEXT("UFogVisibilityComponent::OnInit() ViewerTeam %hhd"), ViewerTeam->GetTeam());
	}
	FogManager = AFogOfWarManager::Get(this, (uint8)ViewerTeam->GetTeam());

	if (FogManager)
	{
		FogManager->OnUnitVisibilityChanged.AddUObject(this, &UFogVisibilityComponent::OnVisibilityChanged);
		UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit Success"), *GetOwnerString(), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("( %s ) [ %s ] UFogVisibilityComponent::OnInit FogManager is NULL"), *GetOwnerString(), *GetOwner()->GetName());
	}*/
}

void UFogVisibilityComponent::OnVisibilityChanged(AActor* InActor, const bool bNewVisible)
{
	UE_LOG(LogTemp, Error, TEXT(" ( %s ) [ %s ] UFogVisibilityComponent::OnVisibilityChanged"), *GetOwnerString(), *GetOwner()->GetName());
	if (InActor == GetOwner())
	{
		GetOwner()->SetActorTickEnabled(!bNewVisible);
	}
	// TODO: иконка на миникарте
}

FString UFogVisibilityComponent::GetOwnerString()
{
	if (IsServer)
	{
		return TEXT("Server");
	}
	else
	{
		if (IsOwnedByLocalPlayer)
		{
			return TEXT("My");
		}
		else
		{
			return TEXT("Stranger");
		}
	}
}
