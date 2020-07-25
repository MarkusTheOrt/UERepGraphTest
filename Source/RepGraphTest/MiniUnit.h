// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MiniUnit.generated.h"

UCLASS()
class REPGRAPHTEST_API AMiniUnit : public ACharacter
{
	GENERATED_BODY()

public:
	
	AMiniUnit();

	void SetTransformPtr(FTransform* Ptr);

	UFUNCTION(BlueprintImplementableEvent)
    void MoveToPls(const FVector& Location);
	
protected:
	
	virtual void BeginPlay() override;

	FTransform* TransformPtr = nullptr;

	

};
