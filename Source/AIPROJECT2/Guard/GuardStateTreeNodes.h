#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "StateTreeTaskBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeConditionBase.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GuardStateTreeNodes.generated.h"   


//Evaluator
USTRUCT()
struct FGuardContextEvaluatorInstanceData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Category = "Output")
    TObjectPtr<APawn> GuardPawn = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    TObjectPtr<AAIController> GuardController = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    TObjectPtr<AActor> PlayerPawn = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    float DistanceToPlayer = 100000.f;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    TObjectPtr<AActor> PatrolPointA = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    TObjectPtr<AActor> PatrolPointB = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    float AggroDistance = 600.f;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    float LoseDistance = 1200.f;
};

USTRUCT(meta = (DisplayName = "Guard Context"))
struct FGuardContextEvaluator : public FStateTreeEvaluatorCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FGuardContextEvaluatorInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
    void Refresh(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const;
};


USTRUCT()
struct FGuardDistanceConditionInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    float Distance = 0.f;

    UPROPERTY(EditAnywhere, Category = "Input")
    float Threshold = 600.f;

    // true: Distance < Threshold  |  false: Distance > Threshold
    UPROPERTY(EditAnywhere, Category = "Parameter")
    bool bLessThan = true;
};

USTRUCT(meta = (DisplayName = "Guard Distance Check"))
struct FGuardDistanceCondition : public FStateTreeConditionCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FGuardDistanceConditionInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};


//  Wait

USTRUCT()
struct FGuardWaitInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float Duration = 2.f;

    UPROPERTY()
    float Remaining = 0.f;
};

USTRUCT(meta = (DisplayName = "Guard Wait"))
struct FGuardWaitTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FGuardWaitInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};


// Move To (patrulla)
USTRUCT()
struct FGuardMoveToInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AAIController> Controller = nullptr;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float AcceptanceRadius = 100.f;
};

USTRUCT(meta = (DisplayName = "Guard Move To"))
struct FGuardMoveToTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FGuardMoveToInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

//Chase
USTRUCT()
struct FGuardChaseInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Parameter")
    TObjectPtr<AAIController> Controller = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float AcceptanceRadius = 150.f;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float RepathInterval = 0.4f;

    UPROPERTY()
    float Elapsed = 0.f;
};

USTRUCT(meta = (DisplayName = "Guard Chase"))
struct FGuardChaseTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FGuardChaseInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


struct FGuardEQSShared
{
    bool bFinished = false;
    bool bSuccess = false;
    bool bAborted = false;
    FVector Location = FVector::ZeroVector;
};

USTRUCT()
struct FGuardSearchEQSInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AAIController> Controller = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    TObjectPtr<UEnvQuery> QueryTemplate = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float AcceptanceRadius = 100.f;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    float Timeout = 10.f;

    UPROPERTY(VisibleAnywhere, Category = "Output")
    FVector SearchLocation = FVector::ZeroVector;

    UPROPERTY()
    float Elapsed = 0.f;

    UPROPERTY()
    bool bMoving = false;

    int32 RequestId = INDEX_NONE;
    TSharedPtr<FGuardEQSShared> Shared;
};

USTRUCT(meta = (DisplayName = "Guard Search (EQS)"))
struct FGuardSearchEQSTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()
    using FInstanceDataType = FGuardSearchEQSInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};