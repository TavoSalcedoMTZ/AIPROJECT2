#include "GuardStateTreeNodes.h"
#include "GuardCharacter.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"


void FGuardContextEvaluator::Refresh(FStateTreeExecutionContext& Context, FInstanceDataType& Data) const
{
    APawn* Pawn = Cast<APawn>(Context.GetOwner());
    if (!Pawn) { return; }

    Data.GuardPawn = Pawn;
    Data.GuardController = Cast<AAIController>(Pawn->GetController());

    if (!Data.PlayerPawn)
    {
        Data.PlayerPawn = UGameplayStatics::GetPlayerPawn(Pawn, 0);
    }

    if (const AGuardCharacter* Guard = Cast<AGuardCharacter>(Pawn))
    {
        Data.PatrolPointA = Guard->PatrolPointA;
        Data.PatrolPointB = Guard->PatrolPointB;
        Data.AggroDistance = Guard->AggroDistance;
        Data.LoseDistance = Guard->LoseDistance;
    }
}

void FGuardContextEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Refresh(Context, Data);
}

void FGuardContextEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.GuardController || !Data.PlayerPawn)
    {
        Refresh(Context, Data);
    }

    if (Data.GuardPawn && Data.PlayerPawn)
    {
        Data.DistanceToPlayer = Data.GuardPawn->GetDistanceTo(Data.PlayerPawn);
    }
}


bool FGuardDistanceCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
    const FInstanceDataType& Data = Context.GetInstanceData(*this);
    return Data.bLessThan ? (Data.Distance < Data.Threshold) : (Data.Distance > Data.Threshold);
}


EStateTreeRunStatus FGuardWaitTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Data.Remaining = Data.Duration;
    return Data.Remaining <= 0.f ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FGuardWaitTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Data.Remaining -= DeltaTime;
    return Data.Remaining <= 0.f ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}


EStateTreeRunStatus FGuardMoveToTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.Controller || !Data.TargetActor)
    {
        return EStateTreeRunStatus::Failed;
    }

    const EPathFollowingRequestResult::Type Result = Data.Controller->MoveToActor(Data.TargetActor, Data.AcceptanceRadius);

    switch (Result)
    {
    case EPathFollowingRequestResult::Failed:        return EStateTreeRunStatus::Failed;
    case EPathFollowingRequestResult::AlreadyAtGoal: return EStateTreeRunStatus::Succeeded;
    default:                                         return EStateTreeRunStatus::Running;
    }
}

EStateTreeRunStatus FGuardMoveToTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.Controller || !Data.TargetActor)
    {
        return EStateTreeRunStatus::Failed;
    }

    if (Data.Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        const APawn* Pawn = Data.Controller->GetPawn();
        const float Dist = Pawn ? FVector::Dist2D(Pawn->GetActorLocation(), Data.TargetActor->GetActorLocation()) : TNumericLimits<float>::Max();
        return (Dist <= Data.AcceptanceRadius + 150.f) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}

void FGuardMoveToTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    if (Data.Controller) { Data.Controller->StopMovement(); }
}


EStateTreeRunStatus FGuardChaseTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.Controller || !Data.TargetActor)
    {
        return EStateTreeRunStatus::Failed;
    }

    Data.Elapsed = 0.f;
    Data.Controller->MoveToActor(Data.TargetActor, Data.AcceptanceRadius);
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FGuardChaseTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.Controller || !Data.TargetActor)
    {
        return EStateTreeRunStatus::Failed;
    }

    Data.Elapsed += DeltaTime;
    if (Data.Elapsed >= Data.RepathInterval)
    {
        Data.Elapsed = 0.f;
        Data.Controller->MoveToActor(Data.TargetActor, Data.AcceptanceRadius);
    }

    return EStateTreeRunStatus::Running;   
}

void FGuardChaseTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    if (Data.Controller) { Data.Controller->StopMovement(); }
}

=
EStateTreeRunStatus FGuardSearchEQSTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    APawn* Pawn = Data.Controller ? Data.Controller->GetPawn() : nullptr;
    if (!Pawn || !Data.QueryTemplate)
    {
        return EStateTreeRunStatus::Failed;
    }

    Data.Elapsed = 0.f;
    Data.bMoving = false;
    Data.Shared = MakeShared<FGuardEQSShared>();

    TSharedPtr<FGuardEQSShared> Shared = Data.Shared;

    FEnvQueryRequest Request(Data.QueryTemplate, Pawn);
    Data.RequestId = Request.Execute(
        EEnvQueryRunMode::RandomBest25Pct,
        FQueryFinishedSignature::CreateLambda([Shared](TSharedPtr<FEnvQueryResult> Result)
            {
                if (Shared->bAborted) { return; }

                Shared->bFinished = true;
                if (Result.IsValid() && Result->IsSuccessful() && Result->Items.Num() > 0)
                {
                    Shared->Location = Result->GetItemAsLocation(0);
                    Shared->bSuccess = true;
                }
            }));

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FGuardSearchEQSTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.Controller || !Data.Shared.IsValid())
    {
        return EStateTreeRunStatus::Failed;
    }

    Data.Elapsed += DeltaTime;
    if (Data.Elapsed > Data.Timeout)
    {
        return EStateTreeRunStatus::Failed;
    }

    if (!Data.bMoving)
    {
        if (!Data.Shared->bFinished) { return EStateTreeRunStatus::Running; }
        if (!Data.Shared->bSuccess) { return EStateTreeRunStatus::Failed; }

        Data.SearchLocation = Data.Shared->Location;

        const EPathFollowingRequestResult::Type Result = Data.Controller->MoveToLocation(
            Data.SearchLocation, Data.AcceptanceRadius, true, true, true);

        if (Result == EPathFollowingRequestResult::Failed) { return EStateTreeRunStatus::Failed; }
        if (Result == EPathFollowingRequestResult::AlreadyAtGoal) { return EStateTreeRunStatus::Succeeded; }

        Data.bMoving = true;
        return EStateTreeRunStatus::Running;
    }

    if (Data.Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        const APawn* Pawn = Data.Controller->GetPawn();
        const float Dist = Pawn ? FVector::Dist2D(Pawn->GetActorLocation(), Data.SearchLocation) : TNumericLimits<float>::Max();
        return (Dist <= Data.AcceptanceRadius + 150.f) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}

void FGuardSearchEQSTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (Data.Shared.IsValid())
    {
        Data.Shared->bAborted = true;
        Data.Shared.Reset();
    }

    if (Data.RequestId != INDEX_NONE && Data.Controller)
    {
        if (UEnvQueryManager* Manager = UEnvQueryManager::GetCurrent(Data.Controller->GetWorld()))
        {
            Manager->AbortQuery(Data.RequestId);
        }
    }
    Data.RequestId = INDEX_NONE;

    if (Data.Controller) { Data.Controller->StopMovement(); }
    Data.Elapsed = 0.f;
    Data.bMoving = false;
}