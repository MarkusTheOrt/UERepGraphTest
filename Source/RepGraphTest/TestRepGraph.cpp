// Fill out your copyright notice in the Description page of Project Settings.


#include "TestRepGraph.h"

#include "Unit.h"
#include "Engine/LevelScriptActor.h"
#include "GameFramework/PlayerState.h"


void InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, float ServerMaxTickRate)
{
	AActor* CDO = Class->GetDefaultObject<AActor>();

	Info.ReplicationPeriodFrame = FMath::Max<uint32>( (uint32)FMath::RoundToFloat(ServerMaxTickRate / CDO->NetUpdateFrequency), 1);

	UClass* NativeClass = Class;
	while(!NativeClass->IsNative() && NativeClass->GetSuperClass() && NativeClass->GetSuperClass() != AActor::StaticClass())
	{
		NativeClass = NativeClass->GetSuperClass();
	}
}

void UTestRepGraph::InitGlobalGraphNodes()
{
	UE_LOG(LogTemp, Warning, TEXT("REP Graph Init Nodes"));
	PreAllocateRepList(3, 12);
	PreAllocateRepList(6, 12);
	PreAllocateRepList(128, 64);
	PreAllocateRepList(512, 16);

	// -----------------------------------------------------------------------------------------------------------------
	// Spawn our Nodes

	DynNode = CreateNewNode<UTestRepGraphNode_DynamicRepNode>();

	AddGlobalGraphNode(DynNode);
	
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	
	AddGlobalGraphNode(AlwaysRelevantNode);

	const auto PlayerStateNode = CreateNewNode<UTestRepGraphNode_PlayerState_FrequencyLimiter>();
	AddGlobalGraphNode(PlayerStateNode);
}

void UTestRepGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
	Super::InitConnectionGraphNodes(ConnectionManager);
	const auto AlwaysRelevantPerNode = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();

	AddConnectionGraphNode(AlwaysRelevantNode, ConnectionManager);
}

void UTestRepGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();


	auto AddInfo = [&]( UClass* Class, EClassRepPolicies Mapping) { ClassRepNodePolicies.Set(Class, Mapping); };

	
	AddInfo( AUnit::StaticClass(), 						EClassRepPolicies::Units );
	AddInfo( AInfo::StaticClass(), 						EClassRepPolicies::RelevantAllConnections );

	// Not needed
	AddInfo( ALevelScriptActor::StaticClass(), 			EClassRepPolicies::NotRouted );

	// This will be handled by the PlayerState Frequency limiter
	AddInfo( APlayerState::StaticClass(), 				EClassRepPolicies::NotRouted );

	// Special case from UE - not Needed.
	AddInfo( AReplicationGraphDebugActor::StaticClass(),EClassRepPolicies::NotRouted );

	TArray<UClass*> AllReplicatedClasses;

	for(TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());

		// Skip Non-Actor classes and Non-Replicated classes.
		if(!ActorCDO || !ActorCDO->GetIsReplicated())
			continue;

		// Skip SKEL and REINST classes (I have no idea what they are, but this is in the shooter game as well)
		if(Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
			continue;
		

		// This is a replicated Actor class.
		AllReplicatedClasses.Add(Class);

		// Skip the above manually added ones.
		if(ClassRepNodePolicies.Contains(Class, false))
			continue;

		// Filter out classes that do differ from their parents:
		UClass* SuperClass = Class->GetSuperClass();
		if(AActor* SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject()))
		{
			if(		SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated()
				 && SuperCDO->bAlwaysRelevant == ActorCDO->bAlwaysRelevant
				 && SuperCDO->bOnlyRelevantToOwner == ActorCDO->bOnlyRelevantToOwner
				 && SuperCDO->bNetUseOwnerRelevancy == ActorCDO->bNetUseOwnerRelevancy
				 )
				continue;			
		}

		if(ActorCDO->IsA<AUnit>())
		{
			AddInfo(Class, EClassRepPolicies::Units);
		}
		else if(ActorCDO->bAlwaysRelevant && !ActorCDO->bOnlyRelevantToOwner)
		{
			AddInfo(Class, EClassRepPolicies::RelevantAllConnections);
		}
	}

	TArray<UClass*> ExplicitlySetClasses;
	auto SetClassInfo = [&](UClass* Class, const FClassReplicationInfo& Info)
	{
		GlobalActorReplicationInfoMap.SetClassInfo(Class, Info);
		ExplicitlySetClasses.Add(Class);
	};

	FClassReplicationInfo UnitRepInfo;
	UnitRepInfo.DistancePriorityScale = 0.f;
	UnitRepInfo.StarvationPriorityScale = 1.f;
	UnitRepInfo.ActorChannelFrameTimeout = 0;
	
	SetClassInfo( APawn::StaticClass(), UnitRepInfo );
	SetClassInfo( AUnit::StaticClass(), UnitRepInfo );

	UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings.ListSize = 12;

	for(UClass* ReplicatedClass : AllReplicatedClasses)
	{
		if(ExplicitlySetClasses.FindByPredicate(
			[&](const UClass* SetClass) { return ReplicatedClass->IsChildOf(SetClass); }) != nullptr
			)
			continue;

			FClassReplicationInfo ClassInfo;
			InitClassReplicationInfo(ClassInfo, ReplicatedClass, NetDriver->NetServerMaxTickRate);
		
	}
	
}

void UTestRepGraph::ResetGameWorldState()
{
	Super::ResetGameWorldState();
}

EClassRepPolicies UTestRepGraph::GetMappingPolicy(UClass* InClass)
{
	EClassRepPolicies* PolicyPtr = ClassRepNodePolicies.Get(InClass);
	return PolicyPtr ? *PolicyPtr : EClassRepPolicies::NotRouted;
	
}

void UTestRepGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
                                                FGlobalActorReplicationInfo& GlobalInfo)
{
	const EClassRepPolicies MappingPolicy = GetMappingPolicy(ActorInfo.Class);
	switch(MappingPolicy)
	{
		// Don't do anything for there is nothing to do.
		default:
		case EClassRepPolicies::NotRouted:
			break;
	}
}

void UTestRepGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	const EClassRepPolicies MappingPolicy = GetMappingPolicy(ActorInfo.Class);
	switch(MappingPolicy)
	{
		// Don't do anything for there is nothing to do.
		default:
        case EClassRepPolicies::NotRouted:
            break;
	}
}

