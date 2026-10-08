// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "StateTreeTask_FindCover.generated.h"



class UEnvQuery;
class AAIController;


USTRUCT()
struct FStateTreeTask_FindCoverInstanceData
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, Category = "StateTree")
	TObjectPtr<AAIController> AIController;
	UPROPERTY(EditAnywhere, Category = "StateTree")
	TObjectPtr<UEnvQuery> CoverQuery = nullptr;
	UPROPERTY(EditAnywhere, Category = "StateTree")
	FVector CoverLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::SingleResult;  //<--- Tostito estuvo aqui

	int32 QueryRequestID = INDEX_NONE;
	EStateTreeRunStatus ExecutionStatus = EStateTreeRunStatus::Running;
};

USTRUCT(meta = (DisplayName = "Find Cover (EQS)", Category = "AI|Cover"))
struct AIPROJECT2_API FStateTreeTask_FindCover : public FStateTreeTaskCommonBase
{
		GENERATED_BODY()

		FStateTreeTask_FindCover() = default;
		using FInstanceDataType = FStateTreeTask_FindCoverInstanceData;
		virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
		virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext & Context, const FStateTreeTransitionResult & Transition) const override;
		virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext & Context, const float DeltaTime) const override;
};


/**
 * 
 */
class AIPROJECT2_API StateTreeTask_FindCover
{
public:
	StateTreeTask_FindCover();
	~StateTreeTask_FindCover();
};
