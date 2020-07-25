// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"


#include "AIController.h"
#include "MiniUnit.h"
#include "NavigationSystem.h"

// Sets default values
AUnit::AUnit()
{
 	// Do not tick pls.
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);
	SetReplicatingMovement(true);
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	//if(GetLocalRole() == ROLE_Authority)
	{
		//AddAdditionalUnit(DefaultSpawnClass);
		AddAdditionalUnit(DefaultSpawnClass);
	}
	if(GetLocalRole() >= ROLE_Authority)
	{
		FTimerHandle Delete;
		GetWorldTimerManager().SetTimer(Delete, this, &ThisClass::ChangeTransforms, 5.f, true);
	}
}

void AUnit::ChangeTransforms()
{
	if(GetLocalRole() >= ROLE_Authority)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString(__FUNCTION__));
		for(FTransform& Transform : UnitTransforms)
		{
			const FVector RandVector{FMath::FRand() * 11000.f};
			Transform = FTransform(RandVector);
		}
		UnitTransforms.AddDefaulted();
		
	}
	
}

void AUnit::OnRep_UnitTransforms()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString(__FUNCTION__));
	if(UnitActors.Num() == UnitTransforms.Num())
	{
		for(int i = 0; i < UnitActors.Num(); i++)
		{
			UnitActors[i]->MoveToPls(UnitTransforms[i].GetLocation());
			
		}
	} // Skip this Rep Frame
}

void AUnit::OnUnitSpawned_Implementation(AMiniUnit* Actor)
{
	AMiniUnit* NewUnit = GetWorld()->SpawnActor<AMiniUnit>(DefaultSpawnClass);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, FString(__FUNCTION__));
	NewUnit->SetActorLocation(this->GetActorLocation());
	NewUnit->SpawnDefaultController();
	UnitActors.Add(NewUnit);
	NewUnit->SetTransformPtr(&UnitTransforms.Last());
}

void AUnit::AddAdditionalUnit_Implementation(TSubclassOf<AMiniUnit> NewClass)
{
	if(GetLocalRole() < ROLE_Authority) return;
	AMiniUnit* NewUnit = GetWorld()->SpawnActor<AMiniUnit>(NewClass);
	NewUnit->SetActorLocation(this->GetActorLocation());
	NewUnit->SpawnDefaultController();
	UnitTransforms.AddDefaulted();
	NewUnit->SetTransformPtr(&UnitTransforms.Last());
	
	UnitActors.Add(NewUnit);
	OnUnitSpawned(NewUnit);
	
}

void AUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnit, UnitTransforms);
	
}


