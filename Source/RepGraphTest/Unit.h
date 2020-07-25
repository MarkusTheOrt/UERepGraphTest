// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Unit.generated.h"



UCLASS()
class REPGRAPHTEST_API AUnit : public AActor
{
	GENERATED_BODY()

public:
	
	AUnit();

protected:
	
	virtual void BeginPlay() override;

	// Lets try not to replicate this one for now.
	UPROPERTY()
	TArray<AActor*> UnitActors;
	
	UPROPERTY(Replicated)
	TArray<FTransform> UnitTransforms;
	
public:	

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutProps)const override;
	
	
};
