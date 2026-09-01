#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ResetThrowPattern.generated.h"

UCLASS()
class RESEASON_API UBTTask_ResetThrowPattern : public UBTTaskNode
{
    GENERATED_BODY()

    public:
    UBTTask_ResetThrowPattern();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
