// Fill out your copyright notice in the Description page of Project Settings.


#include "TestRepGraph.h"

#include "EngineUtils.h"
#include "Unit.h"



void UTestRepGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();
	UE_LOG(LogTemp, Warning, TEXT("RepGraph Started"));
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());

		if(!ActorCDO || !ActorCDO->GetIsReplicated())
		{
			continue;
		}

		if(Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}

		
        // This is old.
        if (ActorCDO->IsA<AUnit>())
        {
	        FClassReplicationInfo UnitInfo;
        	UnitInfo.ReplicationPeriodFrame = FMath::Max<uint32>(NetDriver->NetServerMaxTickRate / ActorCDO->NetUpdateFrequency, 1);
        	GlobalActorReplicationInfoMap.SetClassInfo(Class, UnitInfo);
        	
        } else
        {
        	
        	FClassReplicationInfo ClassInfo;

        	ClassInfo.ReplicationPeriodFrame = FMath::Max<uint32>(
                (uint32)FMath::RoundToFloat(NetDriver->NetServerMaxTickRate / ActorCDO->NetUpdateFrequency), 1);
        	GlobalActorReplicationInfoMap.SetClassInfo(Class, ClassInfo);
        }
        
		
		
	}
	
}

void UTestRepGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	PreAllocateRepList(3, 12);
	PreAllocateRepList(6, 12);
	PreAllocateRepList(128, 64);
	PreAllocateRepList(4096, 2);

	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);
	

	UnitNodes = CreateNewNode<UTestRepGraphNode_Units>();
	AddGlobalGraphNode(UnitNodes);
	
}

void UTestRepGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	auto AlwaysRelevantNodeForConnection = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
	AddConnectionGraphNode(AlwaysRelevantNodeForConnection, ConnectionManager);

	AlwaysRelevantForConnectionNodes.Add(ConnectionManager->NetConnection, AlwaysRelevantNodeForConnection);
}

void UTestRepGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
	FGlobalActorReplicationInfo& GlobalInfo)
{
	if(ActorInfo.Class->IsChildOf<AUnit>())
	{
		UnitNodes->NotifyAddNetworkActor(ActorInfo);
	}
	else if(ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
	}
	else if(ActorInfo.Actor->bOnlyRelevantToOwner)
	{
		if(auto Node = AlwaysRelevantForConnectionNodes.FindRef(ActorInfo.Actor->GetNetConnection()))
		{
			Node->NotifyAddNetworkActor(FNewReplicatedActorInfo(ActorInfo.Actor));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s - No Connection Match for %s"),
				*FString(__FUNCTION__), *ActorInfo.Actor->GetName());

			// Leave no one behind.
			AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
		}
	}
}

void UTestRepGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	if(ActorInfo.Class == AUnit::StaticClass())
	{
		UnitNodes->NotifyRemoveNetworkActor(ActorInfo, false);
	}
	else if(ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
	}
	else if(ActorInfo.Actor->bOnlyRelevantToOwner)
	{
		if(auto Node = AlwaysRelevantForConnectionNodes.FindRef(ActorInfo.Actor->GetNetConnection()))
		{
			Node->NotifyRemoveNetworkActor(ActorInfo);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s - No Connection Match for %s"),
				*FString(__FUNCTION__), *ActorInfo.Actor->GetName());
		}
	}
}

int32 UTestRepGraph::ServerReplicateActors(float DeltaSeconds)
{
	return Super::ServerReplicateActors(DeltaSeconds);
}

UTestRepGraphNode_Units::UTestRepGraphNode_Units()
{
	//bRequiresPrepareForReplicationCall = true;
}


void UTestRepGraphNode_Units::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	
	// Count to max and go back to 0
	if(ActorsToReplicate.Num() == 0)
		return;
	const int i = FMath::Min(ActorsToReplicate.Num()-1, FMath::Max(LastIndex++, 0));
	if(LastIndex +1 == ActorsToReplicate.Num()) LastIndex = 0;
	Params.OutGatheredReplicationLists.AddReplicationActorList(ActorsToReplicate[i]);
}

void UTestRepGraphNode_Units::NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor)
{
	if(ActorsToReplicate.Num() == 0 || ActorsToReplicate.Last().Num() == TargetActorsPerFrame)
	{
		ActorsToReplicate.AddDefaulted();
		UE_LOG(LogTemp, Warning, TEXT("Created new Bucket with Index of %d"), ActorsToReplicate.Num() -1);
	}
	ActorsToReplicate.Last().PrepareForWrite();
	ActorsToReplicate.Last().ConditionalAdd(Actor.Actor);
}

bool UTestRepGraphNode_Units::NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& Actor, bool bWarnIfNotFound)
{
	for(auto& List : ActorsToReplicate)
	{
		if(List.Contains(Actor.Actor))
		{
			List.Remove(Actor.Actor);
			return true;
		}
	}
	return false;
}

void UTestRepGraphNode_Units::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
{
	DebugInfo.Log(NodeName);
	DebugInfo.PushIndent();

	int32 i = 0;
	for(const auto& List : ActorsToReplicate)
	{
		LogActorRepList(DebugInfo, FString::Printf(TEXT("Bucket [%d]"), i++), List);
	}
	DebugInfo.PopIndent();
}
