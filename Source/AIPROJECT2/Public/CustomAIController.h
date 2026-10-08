// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Components/StateTreeAIComponent.h"
#include "CustomAIController.generated.h"

/**
 * 
 */
UCLASS()
class AIPROJECT2_API ACustomAIController : public AAIController
{
	GENERATED_BODY()
	

public : 

	ACustomAIController();


protected : 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;


};
