// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "TestRepGraph.generated.h"


/**
 * The Test Replication Graph
 * This is not doing any spatialization or view culling based off of Fog of War, the reason being that this simulates
 * the worst-case scenario for the Replication in our given scenario. We're trying to aim for 2700 Units Replicating)
 * which would simulate a maxed out 4vs4 game.
 */
UCLASS(Transient, Config = Engine)
class REPGRAPHTEST_API UTestRepGraph : public UReplicationGraph
{
	GENERATED_BODY()


	// ---------------------------
	// UReplicationGraph Overrides

	virtual void InitGlobalActorClassSettings() override;	
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
	                                         FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;

private:

	/** Actors that are always replicated, no condition. */
	UPROPERTY()
	UReplicationGraphNode_ActorList* AlwaysRelevantNode;

	/** Actors that are always replicated for specific connections. */
	UPROPERTY()
	TMap<UNetConnection*, UReplicationGraphNode_AlwaysRelevant_ForConnection*> AlwaysRelevantForConnectionNodes;

	UPROPERTY()
	class UTestRepGraphNode_Units* UnitNodes;
};


/**
 *
 * **************
 * How this works
 * **************
 *
 * Units are divided into buckets of max value (see TargetActorsPerFrame below).
 * In the worst case we have approx. 2700 units (4v4 with 1000 CP)  
 * Each frame One bucket gets replicated.
 * so first frame would be the first bucket, second frame - second bucket and so on...
 * Once we're done with all buckets we start back at the first one
 * By doing so I hope this won't kill the bandwidth while keeping the CPU cool and keeping the game playable with an
 * reasonable responsiveness
 *
 * *****
 * Units
 * *****
 *
 * Units in this project are represented by an ACharacter walking around. The Server will give these units different
 * Tasks (AI Move To)
 * and Replicate their movement onto the client. We can then consider the playability and also record the Network
 * bandwidth.
 * 
 * - Units are generally considered for replication.
 * - Units in idle are not considered for replication. - This is not implemented into this Demo project
 * (to ensure we're in worst case scenario)
 * 
 */
UCLASS(Config = Engine)
class UTestRepGraphNode_Units : public UReplicationGraphNode
{
	GENERATED_BODY()

	UTestRepGraphNode_Units();


public:
	//virtual void PrepareForReplication() override;

	// Chooses which of the buckets is being replicated to the users.
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	// Adds an Actor to an bucket - This needs improvement
	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override;

	// Removes an actors from its specific bucket - This needs improvement
	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& Actor, bool bWarnIfNotFound) override;

	// Prints all the buckets to debug (use Net.Repgraph.PrintAll)
	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;
	
private:

	// If one Actor represents 20 - 30 Units this might equate up to 1200 units per frame.
	// How many units do we want to be transmitted per frame?
	UPROPERTY(Config)
	int32 TargetActorsPerFrame = 40;

	// Buckets
	TArray<FActorRepListRefView> ActorsToReplicate;

	// The Last bucket index that was being replicated.
	int32 LastIndex = 0;
	
};