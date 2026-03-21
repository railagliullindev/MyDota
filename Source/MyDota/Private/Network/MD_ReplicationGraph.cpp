// Rail Agliullin Dev. All Rights Reserved

#include "Network/MD_ReplicationGraph.h"

#include "EngineUtils.h"
#include "Characters/MD_CharacterBase.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Systems/FogOfWar/FogOfWarManager.h"

UReplicationGraphNode_AlwaysRelevant_WithPending::UReplicationGraphNode_AlwaysRelevant_WithPending()
{
	// Вызывайте PrepareForReplication перед репликацией один раз за кадр
	bRequiresPrepareForReplicationCall = true;
}

void UReplicationGraphNode_AlwaysRelevant_WithPending::PrepareForReplication()
{
	UMD_ReplicationGraph* ReplicationGraph = Cast<UMD_ReplicationGraph>(GetOuter());
	ReplicationGraph->HandlePendingActorsAndTeamRequests();
}

void UReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	UMD_ReplicationGraph* ReplicationGraph = Cast<UMD_ReplicationGraph>(GetOuter());
	const UMD_ConnectionManager* ConnectionManager = Cast<UMD_ConnectionManager>(&Params.ConnectionManager);

	if (ReplicationGraph && ConnectionManager && ConnectionManager->Team != -1)
	{
		// Добавляем FogManager для этой команды
		if (AFogOfWarManager* TeamFog = TeamFogManagers.FindRef(static_cast<uint8>(ConnectionManager->Team)))
		{
			// Создаём временный список и добавляем актор
			FActorRepListRefView RepList;
			RepList.Add(TeamFog);

			Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
		}

		// Добавляем всех членов своей команды
		if (TArray<UMD_ConnectionManager*>* TeamConnections = ReplicationGraph->TeamConnectionListMap.GetConnectionArrayForTeam(ConnectionManager->Team))
		{
			for (const UMD_ConnectionManager* TeamMember : *TeamConnections)
			{
				if (TeamMember && TeamMember->TeamConnectionNode)
				{
					TeamMember->TeamConnectionNode->GatherActorListsForConnectionDefault(Params);
				}
			}
		}

		// Добавляем видимых членов вражеской команды
		const TArray<UMD_ConnectionManager*>& NonTeamConnections = ReplicationGraph->TeamConnectionListMap.GetVisibleConnectionArrayForNonTeam(ConnectionManager->Pawn.Get(), ConnectionManager->Team);

		for (const UMD_ConnectionManager* NonTeamMember : NonTeamConnections)
		{
			if (NonTeamMember && NonTeamMember->TeamConnectionNode)
			{
				NonTeamMember->TeamConnectionNode->GatherActorListsForConnectionDefault(Params);
			}
		}
	}
	else
	{
		GatherActorListsForConnectionDefault(Params);
	}
}

void UReplicationGraphNode_AlwaysRelevant_ForTeam::AddFogManagerForTeam(uint8 TeamID, AFogOfWarManager* FogManager)
{
	if (FogManager && !TeamFogManagers.Contains(TeamID))
	{
		TeamFogManagers.Add(TeamID, FogManager);
		FogManagerToTeam.Add(FogManager, TeamID);

		// Добавляем в общий список акторов узла
		FNewReplicatedActorInfo ActorInfo(FogManager);
		NotifyAddNetworkActor(ActorInfo);
	}
}

void UReplicationGraphNode_AlwaysRelevant_ForTeam::RemoveFogManagerForTeam(uint8 TeamID)
{
	if (AFogOfWarManager* FogManager = TeamFogManagers.FindRef(TeamID))
	{
		FogManagerToTeam.Remove(FogManager);
		TeamFogManagers.Remove(TeamID);

		// Удаляем из списка акторов узла
		FNewReplicatedActorInfo ActorInfo(FogManager);
		NotifyRemoveNetworkActor(ActorInfo);
	}
}

void UReplicationGraphNode_AlwaysRelevant_ForTeam::ClearAllFogManagers()
{
	for (auto& Pair : TeamFogManagers)
	{
		if (AFogOfWarManager* FogManager = Pair.Value)
		{
			FNewReplicatedActorInfo ActorInfo(FogManager);
			NotifyRemoveNetworkActor(ActorInfo);
		}
	}

	TeamFogManagers.Empty();
	FogManagerToTeam.Empty();
}

void UReplicationGraphNode_AlwaysRelevant_ForTeam::GatherActorListsForConnectionDefault(const FConnectionGatherActorListParameters& Params)
{
	Super::GatherActorListsForConnection(Params);
}

TArray<UMD_ConnectionManager*>* FTeamConnectionListMap::GetConnectionArrayForTeam(int32 Team)
{
	return Find(Team);
}

TArray<UMD_ConnectionManager*> FTeamConnectionListMap::GetVisibleConnectionArrayForNonTeam(const APawn* Pawn, int32 Team)
{
	TArray<UMD_ConnectionManager*> VisibleConnections;

	if (!IsValid(Pawn))
	{
		return VisibleConnections;
	}

	const UWorld* World = Pawn->GetWorld();
	AFogOfWarManager* FogOfWarManager = AFogOfWarManager::Get(World, Team);

	if (!FogOfWarManager)
	{
		return VisibleConnections;
	}

	// Получаем позицию наблюдателя
	FVector ObserverLocation = Pawn->GetActorLocation();

	// Проверяем, видит ли наблюдатель врагов (его собственная клетка видима)
	bool bObserverCanSee = FogOfWarManager->IsCellVisible(ObserverLocation);

	if (!bObserverCanSee)
	{
		// Если наблюдатель в тумане, он не видит врагов
		return VisibleConnections;
	}

	// Ищем всех игроков не своей команды
	TArray<int32> Teams;
	GetKeys(Teams);

	for (int32 OtherTeamID : Teams)
	{
		if (OtherTeamID == Team) continue; // Пропускаем свою команду

		if (const TArray<UMD_ConnectionManager*>* OtherTeamMembers = GetConnectionArrayForTeam(OtherTeamID))
		{
			for (UMD_ConnectionManager* EnemyConnection : *OtherTeamMembers)
			{
				if (!EnemyConnection->Pawn.IsValid())
				{
					continue;
				}

				APawn* EnemyPawn = EnemyConnection->Pawn.Get();
				FVector EnemyLocation = EnemyPawn->GetActorLocation();

				// Проверяем, виден ли враг через Fog of War
				if (FogOfWarManager->IsCellVisible(EnemyLocation))
				{
					VisibleConnections.Add(EnemyConnection);
				}
			}
		}
	}

	return VisibleConnections;
}

void FTeamConnectionListMap::AddConnectionToTeam(int32 Team, UMD_ConnectionManager* ConnManager)
{
	TArray<UMD_ConnectionManager*>& TeamList = FindOrAdd(Team);
	TeamList.Add(ConnManager);
}

void FTeamConnectionListMap::RemoveConnectionFromTeam(int32 Team, UMD_ConnectionManager* ConnManager)
{
	if (TArray<UMD_ConnectionManager*>* TeamList = Find(Team))
	{
		TeamList->RemoveSwap(ConnManager);

		// Удалите команду с карты, если больше нет подключений.
		if (TeamList->Num() == 0)
		{
			Remove(Team);
		}
	}
}

UMD_ReplicationGraph::UMD_ReplicationGraph()
{
	// Укажите используемый класс графа соединений.
	ReplicationConnectionManagerClass = UMD_ConnectionManager::StaticClass();

	AlwaysRelevantClasses.Add(AGameState::StaticClass());
	AlwaysRelevantClasses.Add(APlayerState::StaticClass());
	// AlwaysRelevantClasses.Add(AMD_CharacterBase::StaticClass());
	// AlwaysRelevantClasses.Add(AFogOfWarManager::StaticClass());
}

void UMD_ReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	// Создайте постоянно актуальный узел.
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_WithPending>();
	AddGlobalGraphNode(AlwaysRelevantNode);

	// Не кэшируем FogManager здесь, так как они могут спавниться позже
	// Они будут добавлены через RouteAddNetworkActorToNodes
}

void UMD_ReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	Super::InitConnectionGraphNodes(ConnectionManager);

	// Создайте граф соединений для входящего соединения.
	UMD_ConnectionManager* RepGraph = Cast<UMD_ConnectionManager>(ConnectionManager);

	if (ensure(RepGraph))
	{
		RepGraph->AlwaysRelevantForConnectionNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
		AddConnectionGraphNode(RepGraph->AlwaysRelevantForConnectionNode, ConnectionManager);

		RepGraph->TeamConnectionNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForTeam>();
		AddConnectionGraphNode(RepGraph->TeamConnectionNode, ConnectionManager);

		UE_LOG(LogTemp, Warning, TEXT("!! TRY InitConnectionGraphNodes 1"));
		// Теперь ищем контроллер и назначаем команду
		if (RepGraph->NetConnection)
		{
			UE_LOG(LogTemp, Warning, TEXT("!! TRY InitConnectionGraphNodes 2"));
			APlayerController* PC = RepGraph->NetConnection->GetPlayerController(GetWorld());
			if (PC)
			{
				UE_LOG(LogTemp, Warning, TEXT("!! TRY InitConnectionGraphNodes 3"));
				IFogOfWarTeamInterface* TeamAgent = Cast<IFogOfWarTeamInterface>(PC);
				if (TeamAgent)
				{
					int32 Team = static_cast<int32>(TeamAgent->GetTeam());
					UE_LOG(LogTemp, Warning, TEXT("!! InitConnectionGraphNodes [%s] - %d"), *PC->GetName(), Team);
					UpdateTeamForConnection(RepGraph, Team);
				}
			}
		}
	}
}

void UMD_ReplicationGraph::AddClientConnection(UNetConnection* NetConnection)
{
	Super::AddClientConnection(NetConnection);
}

void UMD_ReplicationGraph::RemoveClientConnection(UNetConnection* NetConnection)
{
	int32 ConnectionId = 0;
	bool bFound = false;

	auto UpdateList = [&](TArray<TObjectPtr<UNetReplicationGraphConnection>>& List)
	{
		for (int32 idx = 0; idx < List.Num(); ++idx)
		{
			UMD_ConnectionManager* ConnectionManager = Cast<UMD_ConnectionManager>(Connections[idx]);
			repCheck(ConnectionManager);

			if (ConnectionManager->NetConnection == NetConnection)
			{
				ensure(!bFound);

				// Удалите соединение с узлом команды, если команда действительна.
				if (ConnectionManager->Team != -1)
				{
					TeamConnectionListMap.RemoveConnectionFromTeam(ConnectionManager->Team, ConnectionManager);
				}

				// Также удалите его из списка входных данных.
				List.RemoveAtSwap(idx, 1, EAllowShrinking::No);
				bFound = true;
			}
			else
			{
				ConnectionManager->ConnectionOrderNum = ConnectionId++;
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
				// Удалить всех всегда актуальных участников
				// Бесшовное перемещение означает, что связи между членами команды останутся актуальными, поскольку контроллеры не будут уничтожены
				ConnectionManager->AlwaysRelevantForConnectionNode->NotifyResetAllNetworkActors();
			}
		}
	};

	EmptyConnectionNode(PendingConnections);
	EmptyConnectionNode(Connections);
}

void UMD_ReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	// Проверяем AlwaysRelevantClasses
	for (const auto& Class : AlwaysRelevantClasses)
	{
		if (ActorInfo.Class->IsChildOf(Class))
		{
			AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
			return;
		}
	}

	// Специальная обработка для FogOfWarManager
	if (AFogOfWarManager* FogManager = Cast<AFogOfWarManager>(ActorInfo.GetActor()))
	{

		// Кэшируем менеджер для быстрого доступа
		uint8 TeamID = static_cast<uint8>(FogManager->AssignedTeamID);
		UE_LOG(LogTemp, Warning, TEXT("!! Spawned FogManager for Team %d"), TeamID);

		CachedFogManagers.Add(TeamID, FogManager);

		// Добавляем FogManager во все существующие соединения этой команды
		if (TArray<UMD_ConnectionManager*>* TeamConnections = TeamConnectionListMap.GetConnectionArrayForTeam(static_cast<int32>(TeamID)))
		{
			UE_LOG(LogTemp, Warning, TEXT("!! Adding FogManager to %d existing connections for Team %d"), TeamConnections->Num(), TeamID);
			for (UMD_ConnectionManager* ConnManager : *TeamConnections)
			{
				if (ConnManager && ConnManager->TeamConnectionNode)
				{
					ConnManager->TeamConnectionNode->NotifyAddNetworkActor(ActorInfo);
					UE_LOG(LogTemp, Warning, TEXT("!! Added FogManager for Team %d to existing connection %s"), TeamID, *ConnManager->GetName());
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("!! No existing connections for Team %d yet"), TeamID);
		}

		return;
	}

	if (AMD_CharacterBase* Character = Cast<AMD_CharacterBase>(ActorInfo.GetActor()))
	{
		UMD_ConnectionManager* ConnectionManager = GetConnectionManagerFromActor(Character);

		if (ConnectionManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("!! RouteAddNetworkActor: Character %s for Team %d, Owner: %s"), *Character->GetName(), ConnectionManager->Team,
				Character->GetOwner() ? *Character->GetOwner()->GetName() : TEXT("None"));

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
		else
		{
			UE_LOG(LogTemp, Error, TEXT("!! RouteAddNetworkActor: No ConnectionManager for Character %s"), *Character->GetName());

			// Если нет ConnectionManager, добавляем в pending
			if (Character->GetOwner())
			{
				PendingConnectionActors.Add(Character);
			}
		}

		return;
	}

	// Если нет, мы проверяем, принадлежит ли оно к какому-либо соединению.
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
		// Добавьте в PendingConnectionActors, если сетевое соединение еще не готово.
		PendingConnectionActors.Add(ActorInfo.GetActor());
	}
}

void UMD_ReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	for (const auto& Class : AlwaysRelevantClasses)
	{
		if (ActorInfo.Class->IsChildOf(Class))
		{
			AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
			return;
		}
	}

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

void UMD_ReplicationGraph::UpdateTeamForConnection(UMD_ConnectionManager* ConnManager, int32 NewTeam)
{
	if (!ConnManager) return;

	UE_LOG(LogTemp, Warning, TEXT("!! UpdateTeamForConnection: Connection %s -> Team %d"), *ConnManager->GetName(), NewTeam);

	// Очистка старой команды
	if (ConnManager->Team != -1)
	{
		if (AFogOfWarManager* OldMgr = CachedFogManagers.FindRef(static_cast<uint8>(ConnManager->Team)))
		{
			ConnManager->TeamConnectionNode->NotifyRemoveNetworkActor(FNewReplicatedActorInfo(OldMgr));
		}
		TeamConnectionListMap.RemoveConnectionFromTeam(ConnManager->Team, ConnManager);
	}

	ConnManager->Team = NewTeam;
	TeamConnectionListMap.AddConnectionToTeam(NewTeam, ConnManager);

	// Добавляем FogManager для новой команды
	if (NewTeam != -1)
	{
		if (AFogOfWarManager* TeamFog = GetFogManager(static_cast<uint8>(NewTeam)))
		{
			UE_LOG(LogTemp, Warning, TEXT("!! Adding existing FogManager to new connection for Team %d"), NewTeam);
			ConnManager->TeamConnectionNode->NotifyAddNetworkActor(FNewReplicatedActorInfo(TeamFog));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("!! FogManager for Team %d not yet spawned"), NewTeam);
		}
	}
}

void UMD_ReplicationGraph::SetTeamForPlayerController(APlayerController* PlayerController, int32 Team)
{
	if (PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("!! SetTeamForPlayerController: PC %s, Team %d"), *PlayerController->GetName(), Team);

		if (UMD_ConnectionManager* ConnectionManager = GetConnectionManagerFromActor(PlayerController))
		{
			const int32 CurrentTeam = ConnectionManager->Team;
			if (CurrentTeam != Team)
			{
				// Удалите связь со старым списком команд.
				if (CurrentTeam != -1)
				{
					TeamConnectionListMap.RemoveConnectionFromTeam(CurrentTeam, ConnectionManager);
				}

				// Добавьте график в новый список команд.
				if (Team != -1)
				{
					UE_LOG(LogTemp, Warning, TEXT("!! SetTeamForPlayerController [%s] - %d"), *PlayerController->GetName(), Team);
					TeamConnectionListMap.AddConnectionToTeam(Team, ConnectionManager);
				}
				ConnectionManager->Team = Team;
			}
		}
		else
		{
			// Добавьте в PendingTeamRequests, если подключение к сети еще не готово
			PendingTeamRequests.Emplace(Team, PlayerController);
		}
	}
}

void UMD_ReplicationGraph::HandlePendingActorsAndTeamRequests()
{
	// Настройте все ожидающие запросы команды.
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

	// Настройте все ожидающие подключения.
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

void UMD_ReplicationGraph::AddFogManagerToConnection(UMD_ConnectionManager* ConnManager, uint8 TeamID)
{
	if (!ConnManager || !ConnManager->TeamConnectionNode) return;

	if (AFogOfWarManager* TeamFog = GetFogManager(TeamID))
	{
		FNewReplicatedActorInfo ActorInfo(TeamFog);
		ConnManager->TeamConnectionNode->NotifyAddNetworkActor(ActorInfo);
		UE_LOG(LogTemp, Warning, TEXT("!! Added FogManager for Team %d to connection %s"), TeamID, *ConnManager->GetName());
	}
}

AFogOfWarManager* UMD_ReplicationGraph::GetFogManager(uint8 TeamID)
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

	// 1. Пробуем получить NetConnection напрямую
	if (UNetConnection* NetConnection = Actor->GetNetConnection())
	{
		return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(NetConnection));
	}

	// 2. Если нет прямого соединения, пробуем через Owner
	if (AActor* Owner = Actor->GetOwner())
	{
		if (UNetConnection* OwnerConnection = Owner->GetNetConnection())
		{
			return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(OwnerConnection));
		}

		// 3. Если Owner - PlayerController, пробуем через его PlayerState
		if (APlayerController* PC = Cast<APlayerController>(Owner))
		{
			if (APlayerState* PS = PC->PlayerState)
			{
				if (UNetConnection* PSConnection = PS->GetNetConnection())
				{
					return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(PSConnection));
				}
			}
		}
	}

	// 4. Если Actor - Pawn, пробуем через Controller
	if (const APawn* Pawn = Cast<APawn>(Actor))
	{
		if (AController* Controller = Pawn->GetController())
		{
			if (UNetConnection* ControllerConnection = Controller->GetNetConnection())
			{
				return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(ControllerConnection));
			}

			// Если Controller - PlayerController, пробуем через его PlayerState
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				if (APlayerState* PS = PC->PlayerState)
				{
					if (UNetConnection* PSConnection = PS->GetNetConnection())
					{
						return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(PSConnection));
					}
				}
			}
		}
	}

	// 5. Если Actor - Controller, пробуем напрямую
	if (const AController* Controller = Cast<AController>(Actor))
	{
		if (UNetConnection* ControllerConnection = Controller->GetNetConnection())
		{
			return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(ControllerConnection));
		}

		// Если Controller - PlayerController, пробуем через его PlayerState
		if (const APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (APlayerState* PS = PC->PlayerState)
			{
				if (UNetConnection* PSConnection = PS->GetNetConnection())
				{
					return Cast<UMD_ConnectionManager>(FindOrAddConnectionManager(PSConnection));
				}
			}
		}
	}

	return nullptr;
}
