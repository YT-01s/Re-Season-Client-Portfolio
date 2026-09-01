#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ComboAttack.generated.h"

UCLASS()
class RESEASON_API UBTTask_ComboAttack : public UBTTaskNode
{
    GENERATED_BODY()

    public:
    UBTTask_ComboAttack();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
