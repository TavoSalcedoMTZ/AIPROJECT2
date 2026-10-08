#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuardCharacter.generated.h"

class UStateTreeComponent;

UCLASS()
class AIPROJECT2_API AGuardCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AGuardCharacter();

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Guard|Patrol")
    TObjectPtr<AActor> PatrolPointA = nullptr;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Guard|Patrol")
    TObjectPtr<AActor> PatrolPointB = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guard|Perception")
    float AggroDistance = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guard|Perception")
    float LoseDistance = 1200.f;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Guard")
    TObjectPtr<UStateTreeComponent> StateTreeComp;
};