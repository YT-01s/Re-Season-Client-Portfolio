#include "BTTask_ComboAttack.h"
#include "AIController.h"
#include "StoneBossCharacter.h"

UBTTask_ComboAttack::UBTTask_ComboAttack()
{
    NodeName = TEXT("COMBO ATTACK");
}

EBTNodeResult::Type UBTTask_ComboAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        if (AStoneBossCharacter* Boss = Cast<AStoneBossCharacter>(AICon->GetPawn()))
        {
            //AICon->StopMovement();
            Boss->PlayComboAttackMontage();   // ÄÞº¸ ¸ùÅ¸ÁÖ ½ÇÇà
            return EBTNodeResult::Succeeded;
        }
    }
    return EBTNodeResult::Failed;
}
