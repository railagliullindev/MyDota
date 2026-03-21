// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "Widgets/InGame/HeroStatusWidget.h"
#include "MD_ReplicationGraph.generated.h"

class AMD_CharacterBase;
class AFogOfWarManager;
class UMD_ReplicationGraphConnection;
class UFogOfWarNode;

UCLASS()
class MYDOTA_API UReplicationGraphNode_AlwaysRelevant_WithPending : public UReplicationGraphNode_ActorList
{
	GENERATED_BODY()

public:

	UReplicationGraphNode_AlwaysRelevant_WithPending();

protected:

	virtual void PrepareForReplication() override;
};

UCLASS()
class MYDOTA_API UReplicationGraphNode_AlwaysRelevant_ForTeam : public UReplicationGraphNode_ActorList
{
	GENERATED_BODY()
public:

	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	// Добавляем FogManager для конкретной команды
	void AddFogManagerForTeam(uint8 TeamID, AFogOfWarManager* FogManager);

	void RemoveFogManagerForTeam(uint8 TeamID);

	void ClearAllFogManagers();

protected:

	virtual void GatherActorListsForConnectionDefault(const FConnectionGatherActorListParameters& Params);

private:

	// Храним FogManagers отдельно для каждой команды - используем TMap с TObjectPtr
	UPROPERTY()
	TMap<uint8, TObjectPtr<AFogOfWarManager>> TeamFogManagers;

	// Для обратного поиска (опционально)
	UPROPERTY()
	TMap<TWeakObjectPtr<AFogOfWarManager>, uint8> FogManagerToTeam;
};

UCLASS()
class UMD_ConnectionManager : public UNetReplicationGraphConnection
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantForConnectionNode;

	UPROPERTY()
	UReplicationGraphNode_AlwaysRelevant_ForTeam* TeamConnectionNode;

	int32 Team = -1;

	UPROPERTY()
	TWeakObjectPtr<APawn> Pawn = nullptr;

	bool IsTeamAssigned() const
	{
		return Team != -1;
	}
};

struct FTeamConnectionListMap : TMap<int32, TArray<UMD_ConnectionManager*>>
{
	TArray<UMD_ConnectionManager*>* GetConnectionArrayForTeam(int32 Team);
	TArray<UMD_ConnectionManager*> GetVisibleConnectionArrayForNonTeam(const APawn* Pawn, int32 Team);

	void AddConnectionToTeam(int32 Team, UMD_ConnectionManager* ConnManager);
	void RemoveConnectionFromTeam(int32 Team, UMD_ConnectionManager* ConnManager);
};

UCLASS()
class MYDOTA_API UMD_ReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:

	UMD_ReplicationGraph();

	void SetTeamForPlayerController(APlayerController* PlayerController, int32 Team);

	void HandlePendingActorsAndTeamRequests();

	// Добавляем метод для добавления FogManager к соединению
	void AddFogManagerToConnection(UMD_ConnectionManager* ConnManager, uint8 TeamID);

protected:

	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;

	virtual void AddClientConnection(UNetConnection* NetConnection) override;
	virtual void RemoveClientConnection(UNetConnection* NetConnection) override;

	virtual void ResetGameWorldState() override;

	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	void UpdateTeamForConnection(UMD_ConnectionManager* ConnManager, int32 NewTeam);

private:

	TArray<UClass*> AlwaysRelevantClasses;

	UPROPERTY()
	UReplicationGraphNode_AlwaysRelevant_WithPending* AlwaysRelevantNode;

	UPROPERTY()
	TArray<AActor*> PendingConnectionActors;
	TArray<TTuple<int32, APlayerController*>> PendingTeamRequests;

	friend UReplicationGraphNode_AlwaysRelevant_ForTeam;

	FTeamConnectionListMap TeamConnectionListMap;

	UPROPERTY()
	TMap<uint8, AFogOfWarManager*> CachedFogManagers;

	AFogOfWarManager* GetFogManager(uint8 TeamID);

	UMD_ConnectionManager* GetConnectionManagerFromActor(const AActor* Actor);
};