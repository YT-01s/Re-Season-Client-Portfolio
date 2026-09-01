#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ThrowWeapon.generated.h"

UCLASS()
class RESEASON_API UBTTask_ThrowWeapon : public UBTTaskNode
{
    GENERATED_BODY()

    public:
    UBTTask_ThrowWeapon();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    int32 RemainingThrows = 0;
    FTimerHandle ThrowTimerHandle;

    void PerformThrow(UBehaviorTreeComponent* OwnerComp);
};
