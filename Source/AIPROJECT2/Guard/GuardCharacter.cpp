#include "GuardCharacter.h"
#include "GuardAIController.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AGuardCharacter::AGuardCharacter()
{
    AIControllerClass = AGuardAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    StateTreeComp = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
}