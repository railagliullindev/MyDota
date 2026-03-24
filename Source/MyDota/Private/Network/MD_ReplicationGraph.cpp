// Rail Agliullin Dev. All Rights Reserved

#include "Network/MD_ReplicationGraph.h"

#include "EngineUtils.h"
#include "Characters/MD_CharacterBase.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Pawns/MD_CameraPawn.h"
#include "Systems/FogOfWar/FogOfWarManager.h"

//=============================================================================
// UReplicationGraphNode_AlwaysRelevant_WithPending
//=============================================================================

UReplicationGraphNode_AlwaysRelevant_WithPending::UReplicationGraphNode_AlwaysRelevant_WithPending()
{
	bRequiresPrepareForReplicationCall = true;
}

void UReplicationGraphNode_AlwaysRelevant_WithPending::PrepareForReplication()
{
	if (UMD_ReplicationGraph* ReplicationGraph = Cast<UMD_ReplicationGraph>(GetOuter()))
	{
		ReplicationGraph->HandlePendingActorsAndTeamRequests();
	}
}

void UReplicationGraphNode_FogOfWarManager::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	const UMD_ConnectionManager* ConnectionManager = Cast<UMD_ConnectionManager>(&Params.ConnectionManager);

	if (!ConnectionManager || ConnectionManager->TeamId == -1)
	{
		return;
	}

	// Если команда соединения совпадает с командой FogManager — отдаём
	if (ConnectionManager->TeamId == MyTeamID && MyFogManager)
	{
		FActorRepListRefView RepList;
		RepList.Add(MyFogManager);
		Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
	}
}

void UReplicationGraphNode_FogOfWarManager::SetFogManager(uint8 TeamID, AFogOfWarManager* FogManager)
{
	MyTeamID = TeamID;
	MyFogManager = FogManager;
}

//=============================================================================
// UReplicationGraphNode_AlwaysRelevant_ForTeam
//=============================================================================

void UReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	UMD_ReplicationGraph* ReplicationGraph = Cast<UMD_ReplicationGraph>(GetOuter());
	const UMD_ConnectionManager* ConnectionManager = Cast<UMD_ConnectionManager>(&Params.ConnectionManager);

	if (ReplicationGraph && ConnectionManager && ConnectionManager->TeamId != -1)
	{
		const int32 ObserverTeam = ConnectionManager->TeamId;

		// 1. Добавляем всех членов своей команды
		if (TArray<UMD_ConnectionManager*>* TeamConnections = ReplicationGraph->TeamConnectionListMap.GetConnectionArrayForTeam(ObserverTeam))
		{
			for (const UMD_ConnectionManager* TeamMember : *TeamConnections)
			{
				if (TeamMember && TeamMember->TeamConnectionNode)
				{
					TeamMember->TeamConnectionNode->GatherActorListsForConnectionDefault(Params);
				}
			}
		}

		// 2. Видимые акторы вражеской команды (через туман войны)
		for (const UMD_ConnectionManager* EnemyConnection : ReplicationGraph->TeamConnectionListMap.GetVisibleConnectionArrayForNonTeam(ConnectionManager->Pawn.Get(), ObserverTeam))
		{
			if (EnemyConnection && EnemyConnection->TeamConnectionNode)
			{
				EnemyConnection->TeamConnectionNode->GatherActorListsForConnectionDefault(Params);
			}
		}
	}
	else
	{
		GatherActorListsForConnectionDefault(Params);
	}
}

void UReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnectionDefault(const FConnectionGatherActorListParameters& Params)
{
	Super::GatherActorListsForConnection(Params);
}

TArray<UMD_ConnectionManager*>* FTeamConnectionListMap::GetConnectionArrayForTeam(const int32 TeamId)
{
	return Find(TeamId);
}

//=============================================================================
// FTeamConnectionListMap
//=============================================================================

TArray<UMD_ConnectionManager*> FTeamConnectionListMap::GetVisibleConnectionArrayForNonTeam(const APawn* Observer, const int32 ObserverTeam)
{
	TArray<UMD_ConnectionManager*> VisibleConnections;

	if (!IsValid(Observer))
	{
		return VisibleConnections;
	}

	const UWorld* World = Observer->GetWorld();
	const AFogOfWarManager* FogOfWarManager = AFogOfWarManager::Get(World, ObserverTeam);

	if (!FogOfWarManager)
	{
		return VisibleConnections;
	}

	const FVector ObserverLocation = Observer->GetActorLocation();

	// Наблюдатель в тумане войны не видит врагов
	if (!FogOfWarManager->IsCellVisible(ObserverLocation))
	{
		return VisibleConnections;
	}

	TArray<int32> Teams;
	GetKeys(Teams);

	for (const int32 OtherTeamID : Teams)
	{
		if (OtherTeamID == ObserverTeam) continue; // Пропускаем свою команду

		if (const TArray<UMD_ConnectionManager*>* OtherTeamMembers = GetConnectionArrayForTeam(OtherTeamID))
		{
			for (UMD_ConnectionManager* EnemyConnection : *OtherTeamMembers)
			{
				if (!EnemyConnection->Pawn.IsValid())
				{
					continue;
				}

				if (const FVector EnemyLocation = EnemyConnection->Pawn->GetActorLocation(); FogOfWarManager->IsCellVisible(EnemyLocation))
				{
					VisibleConnections.Add(EnemyConnection);
				}
			}
		}
	}

	return VisibleConnections;
}

void FTeamConnectionListMap::AddConnectionToTeam(const int32 TeamId, UMD_ConnectionManager* ConnManager)
{
	TArray<UMD_ConnectionManager*>& TeamList = FindOrAdd(TeamId);
	TeamList.Add(ConnManager);
}

void FTeamConnectionListMap::RemoveConnectionFromTeam(const int32 TeamId, UMD_ConnectionManager* ConnManager)
{
	if (TArray<UMD_ConnectionManager*>* TeamList = Find(TeamId))
	{
		TeamList->RemoveSwap(ConnManager);

		if (TeamList->Num() == 0)
		{
			Remove(TeamId);
		}
	}
}

//=============================================================================
// UMD_ReplicationGraph
//=============================================================================

UMD_ReplicationGraph::UMD_ReplicationGraph()
{
	ReplicationConnectionManagerClass = UMD_ConnectionManager::StaticClass();

	AlwaysRelevantClasses.Add(AGameState::StaticClass());
	AlwaysRelevantClasses.Add(APlayerState::StaticClass());

	IgnoreReplicatesClasses.Add(AMD_CameraPawn::StaticClass());
	IgnoreReplicatesClasses.Add(AFogOfWarManager::StaticClass());
}

void UMD_ReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_WithPending>();
	AddGlobalGraphNode(AlwaysRelevantNode);
}

void UMD_ReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	Super::InitConnectionGraphNodes(ConnectionManager);

	UMD_ConnectionManager* RepGraph = Cast<UMD_ConnectionManager>(ConnectionManager);

	if (ensure(RepGraph))
	{
		RepGraph->AlwaysRelevantForConnectionNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
		AddConnectionGraphNode(RepGraph->AlwaysRelevantForConnectionNode, ConnectionManager);

		RepGraph->TeamConnectionNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForTeam>();
		AddConnectionGraphNode(RepGraph->TeamConnectionNode, ConnectionManager);

		RepGraph->FogNode = CreateNewNode<UReplicationGraphNode_FogOfWarManager>();
		AddConnectionGraphNode(RepGraph->FogNode, ConnectionManager);

		// Назначаем команду, если уже известна
		if (RepGraph->NetConnection)
		{
			if (APlayerController* PC = RepGraph->NetConnection->GetPlayerController(GetWorld()))
			{
				if (const IFogOfWarTeamInterface* TeamAgent = Cast<IFogOfWarTeamInterface>(PC))
				{
					UpdateTeamForConnection(RepGraph, static_cast<int32>(TeamAgent->GetTeam()));
				}
			}
		}
	}
}

void UMD_ReplicationGraph::RemoveClientConnection(UNetConnection* NetConnection)
{
	auto UpdateList = [&](TArray<TObjectPtr<UNetReplicationGraphConnection>>& List)
	{
		for (int32 Idx = 0; Idx < List.Num(); ++Idx)
		{
			if (UMD_ConnectionManager* ConnectionManager = Cast<UMD_ConnectionManager>(List[Idx]))
			{
				if (ConnectionManager->NetConnection == NetConnection)
				{
					if (ConnectionManager->TeamId != -1)
					{
						TeamConnectionListMap.RemoveConnectionFromTeam(ConnectionManager->TeamId, ConnectionManager);
					}
					List.RemoveAtSwap(Idx, 1);
					break;
				}
			}
		}
	};

	UpdateList(Connections);
	UpdateList(PendingConnections);
}

void UMD_ReplicationGraph::ResetGameWorldState()
{
	Super::ResetGameWorldState();

	PendingConnectionActors.Reset();
	PendingTeamRequests.Reset();

	auto EmptyConnectionNode = [](TArray<TObjectPtr<UNetReplicationGraphConnection>>& GraphConnections)
	{
		for (UNetReplicationGraphConnection* GraphConnection : GraphConnections)
		{
			if (const UMD_ConnectionManager* ConnectionManager = Cast<UMD_ConnectionManager>(GraphConnection))
			{
				ConnectionManager->AlwaysRelevantForConnectionNode->NotifyResetAllNetworkActors();
			}
		}
	};

	EmptyConnectionNode(PendingConnections);
	EmptyConnectionNode(Connections);
}

void UMD_ReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	// 0. Игнорируемые классы
	for (const auto& Class : IgnoreReplicatesClasses)
	{
		if (ActorInfo.Class->IsChildOf(Class))
		{
			return;
		}
	}

	// 1. Глобально релевантные акторы
	for (const auto& Class : AlwaysRelevantClasses)
	{
		if (ActorInfo.Class->IsChildOf(Class))
		{
			AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
			return;
		}
	}

	// 2. Герои - добавляем к соединению владельца
	if (AMD_CharacterBase* Character = Cast<AMD_CharacterBase>(ActorInfo.GetActor()))
	{
		if (UMD_ConnectionManager* ConnectionManager = GetConnectionManagerFromActor(Character))
		{
			if (Character->bOnlyRelevantToOwner)
			{
				ConnectionManager->AlwaysRelevantForConnectionNode->NotifyAddNetworkActor(ActorInfo);
			}
			else
			{
				ConnectionManager->TeamConnectionNode->NotifyAddNetworkActor(ActorInfo);

				if (APawn* Pawn = Cast<APawn>(Character))
				{
					ConnectionManager->Pawn = Pawn;
				}
			}
		}
		else if (Character->GetOwner())
		{
			PendingConnectionActors.Add(Character);
		}
		return;
	}

	// 3. Остальные акторы
	if (UMD_ConnectionManager* ConnectionManager = GetConnectionManagerFromActor(ActorInfo.GetActor()))
	{
		if (ActorInfo.Actor->bOnlyRelevantToOwner)
		{
			ConnectionManager->AlwaysRelevantForConnectionNode->NotifyAddNetworkActor(ActorInfo);
		}
		else
		{
			ConnectionManager->TeamConnectionNode->NotifyAddNetworkActor(ActorInfo);

			if (APawn* Pawn = Cast<APawn>(ActorInfo.GetActor()))
			{
				ConnectionManager->Pawn = Pawn;
			}
		}
	}
	else if (ActorInfo.Actor->GetNetOwner())
	{
		PendingConnectionActors.Add(ActorInfo.GetActor());
	}
}

void UMD_ReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	// 1. Глобально релевантные акторы
	for (const auto& Class : AlwaysRelevantClasses)
	{
		if (ActorInfo.Class->IsChildOf(Class))
		{
			AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
			return;
		}
	}

	// 2. Остальные акторы
	if (const UMD_ConnectionManager* ConnectionManager = GetConnectionManagerFromActor(ActorInfo.GetActor()))
	{
		if (ActorInfo.Actor->bOnlyRelevantToOwner)
		{
			ConnectionManager->AlwaysRelevantForConnectionNode->NotifyRemoveNetworkActor(ActorInfo);
		}
		else
		{
			ConnectionManager->TeamConnectionNode->NotifyRemoveNetworkActor(ActorInfo);
		}
	}
	else if (ActorInfo.Actor->GetNetOwner())
	{
		PendingConnectionActors.Remove(ActorInfo.GetActor());
	}
}

void UMD_ReplicationGraph::UpdateTeamForConnection(UMD_ConnectionManager* ConnManager, const int32 NewTeamId)
{
	if (!ConnManager) return;

	// Очистка старой команды (без FogManager)
	if (ConnManager->TeamId != -1)
	{
		TeamConnectionListMap.RemoveConnectionFromTeam(ConnManager->TeamId, ConnManager);
	}

	ConnManager->TeamId = NewTeamId;
	TeamConnectionListMap.AddConnectionToTeam(NewTeamId, ConnManager);

	// Находим FogManager для этой команды на сцене
	if (NewTeamId != -1)
	{
		for (TActorIterator<AFogOfWarManager> It(GetWorld()); It; ++It)
		{
			AFogOfWarManager* Fog = *It;
			if (static_cast<int32>(Fog->AssignedTeamID) == NewTeamId)
			{
				ConnManager->FogNode->SetFogManager(static_cast<uint8>(NewTeamId), Fog);
				break;
			}
		}
	}
}

void UMD_ReplicationGraph::SetTeamForPlayerController(APlayerController* PlayerController, int32 TeamId)
{
	if (!PlayerController) return;

	if (UMD_ConnectionManager* ConnectionManager = GetConnectionManagerFromActor(PlayerController))
	{
		if (ConnectionManager->TeamId != TeamId)
		{
			UpdateTeamForConnection(ConnectionManager, TeamId);
		}
	}
	else
	{
		PendingTeamRequests.Emplace(TeamId, PlayerController);
	}
}

void UMD_ReplicationGraph::HandlePendingActorsAndTeamRequests()
{
	// Обработка отложенных запросов команд
	if (PendingTeamRequests.Num() > 0)
	{
		TArray<TTuple<int32, APlayerController*>> TempRequests = MoveTemp(PendingTeamRequests);

		for (const TTuple<int32, APlayerController*>& Request : TempRequests)
		{
			if (IsValid(Request.Value))
			{
				SetTeamForPlayerController(Request.Value, Request.Key);
			}
		}
	}

	// Обработка отложенных акторов
	if (PendingConnectionActors.Num() > 0)
	{
		TArray<AActor*> PendingActors = MoveTemp(PendingConnectionActors);

		for (AActor* Actor : PendingActors)
		{
			if (IsValid(Actor))
			{
				FGlobalActorReplicationInfo& GlobalInfo = GlobalActorReplicationInfoMap.Get(Actor);
				RouteAddNetworkActorToNodes(FNewReplicatedActorInfo(Actor), GlobalInfo);
			}
		}
	}
}

AFogOfWarManager* UMD_ReplicationGraph::GetFogManager(const uint8 TeamID)
{
	if (AFogOfWarManager** Found = CachedFogManagers.Find(TeamID))
	{
		return *Found;
	}
	return nullptr;
}

UMD_ConnectionManager* UMD_ReplicationGraph::GetConnectionManagerFromActor(const AActor* Actor)
{
	if (!Actor) return nullptr;

	auto GetConnManager = [this](UNetConnection* NetConn) -> UMD_ConnectionManager*
	{
		return NetConn ? Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(NetConn)) : nullptr;
	};

	// 1. Прямое соединение
	if (UNetConnection* DirectConn = Actor->GetNetConnection())
	{
		return GetConnManager(DirectConn);
	}

	// 2. Через Owner
	if (AActor* Owner = Actor->GetOwner())
	{
		if (UNetConnection* OwnerConn = Owner->GetNetConnection())
		{
			return GetConnManager(OwnerConn);
		}

		if (const APlayerController* PC = Cast<APlayerController>(Owner))
		{
			if (const APlayerState* PS = PC->PlayerState)
			{
				if (UNetConnection* PSConn = PS->GetNetConnection())
				{
					return GetConnManager(PSConn);
				}
			}
		}
	}

	// 3. Через Controller (для Pawn)
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (AController* Controller = Pawn->GetController())
		{
			if (UNetConnection* ControllerConn = Controller->GetNetConnection())
			{
				return GetConnManager(ControllerConn);
			}

			if (const APlayerController* PC = Cast<APlayerController>(Controller))
			{
				if (const APlayerState* PS = PC->PlayerState)
				{
					if (UNetConnection* PSConn = PS->GetNetConnection())
					{
						return GetConnManager(PSConn);
					}
				}
			}
		}
	}

	// 4. Сам Actor - Controller
	if (const AController* Controller = Cast<AController>(Actor))
	{
		if (UNetConnection* ControllerConn = Controller->GetNetConnection())
		{
			return GetConnManager(ControllerConn);
		}

		if (const APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (const APlayerState* PS = PC->PlayerState)
			{
				if (UNetConnection* PSConn = PS->GetNetConnection())
				{
					return GetConnManager(PSConn);
				}
			}
		}
	}

	return nullptr;
}
