// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit.h"

// Sets default values
AUnit::AUnit()
{
 	// Do not tick pls.
	PrimaryActorTick.bCanEverTick = false;

	SetReplicates(true);
	SetReplicatingMovement(true);

}

// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUnit::OnUnitSpawned_Implementation(AMiniUnit* Actor)
{
	UnitActors.Add(Actor);
}

void AUnit::AddAdditionalUnit_Implementation(TSubclassOf<AMiniUnit> NewClass)
{
	AMiniUnit* NewUnit = GetWorld()->SpawnActor<AMiniUnit>(NewClass);
	UnitActors.Add(NewUnit);
	UnitTransforms.AddDefaulted();
	OnUnitSpawned(NewUnit);
}

void AUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnit, UnitTransforms);
	
}


