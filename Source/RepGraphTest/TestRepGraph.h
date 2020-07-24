// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "TestRepGraph.generated.h"

enum class EClassRepPolicies : uint8
{
	NotRouted,				// Not Replicated at all
	RelevantAllConnections,	// Always relevant for all connections.

	Buildings,				// Only when in action / being attacked
	Units,					// Varies in Frames (so we can do batches of units)
	Static,					// Does not need to update every frame
	Dynamic,				// Updates every frame
	Dormancy				// Updates dynamically (between static and Dynamic, based on AActor::GetNetDormancy)
};

class UReplicationGraphNode_ActorList;
class UTestRepGraphNode_DynamicRepNode;
class UTestRepGraphNode_AlwaysRelevant_ForConnection;

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


	// -----------------------------------------------------------------------------------------------------------------
	// UReplicationGraph Overrides
	
protected:
	
	virtual void InitGlobalGraphNodes() override;

	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;


	virtual void InitGlobalActorClassSettings() override;
public:
	
	virtual void ResetGameWorldState() override;

	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
	                                         FGlobalActorReplicationInfo& GlobalInfo) override;
	
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	// -----------------------------------------------------------------------------------------------------------------
	// Replication Graph Nodes
	// - These are marked as UPROPERTY so the GC doesn't collect em.

protected:

	UPROPERTY()
	UTestRepGraphNode_DynamicRepNode* DynNode;
	
	UPROPERTY()
	UReplicationGraphNode_ActorList* AlwaysRelevantNode;

	UPROPERTY()
	TArray<UClass*> AlwaysRelevantClasses;

	UPROPERTY()
	TArray<UClass*> UnitClasses;

	UPROPERTY()
	TArray<UClass*> BuildingClasses;

	UPROPERTY()
	TArray<UClass*> NonRTSClasses;
	
protected:
		
	const uint32 MaxUnitBatch = 500ul;

private:

	/**
	* Gets the mapping policy based on class
	* @param InClass - The Class of which is the policy to be determined
	* @returns Policy for the given class.
	*/
	EClassRepPolicies GetMappingPolicy(UClass* InClass);

	TClassMap<EClassRepPolicies> ClassRepNodePolicies;
	
};

/**
 * This Node handles replication of ForConnection relevant things
 * (PlayerController dependent classes)
 */
UCLASS()
class UTestRepGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode
{

	GENERATED_BODY()

public:

	
	
};

/**
 * This is our main Rep Graph Node
 * **************
 * How this works
 * **************
 *
 * Units are divided into batches of max 500. (in worst case we have approx. 2700 units therefore the batches would
 * pan into [500, 500, 500, 500, 500, 200].
 * Each frame One batch gets replicated.
 * so first frame would be the first batch, second frame - second batch and so on...
 * Once we're done (7th Frame) we start again so the order would be 1,2,3,4,5,6,1,2,3,4,5,6.
 * By doing so I hope this won't kill the bandwidth while keeping the CPU cool and keeping the game playable with an
 * reasonable responsiveness (this is for an RTS after all)
 *
 * *********
 * Buildings
 * *********
 *
 * Buildings are not considered for replication unless they are in action (being attacked that is)
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
 * 
 * 
 */
UCLASS()
class UTestRepGraphNode_DynamicRepNode : public UReplicationGraphNode
{
	GENERATED_BODY()
};

/**
 * Limit Player State Frequency to every second frame (no particular reason, but saves a tiny bit of bandwidth)
 */
UCLASS()
class UTestRepGraphNode_PlayerState_FrequencyLimiter : public UReplicationGraphNode
{
	GENERATED_BODY()
};