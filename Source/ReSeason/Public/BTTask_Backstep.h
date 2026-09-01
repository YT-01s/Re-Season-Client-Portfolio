#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Backstep.generated.h"


 // 보스 백스텝 (전용 몽타주 포함)
 
UCLASS()
class RESEASON_API UBTTask_Backstep : public UBTTaskNode
{
    GENERATED_BODY()

    public:
    UBTTask_Backstep();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Latent (지속형) 태스크 — 애니메이션이 끝날 때까지 대기
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    // 현재 몽타주 재생 중인지
    bool bIsMontagePlaying = false;

    // 몽타주 대기 시간
    float MontageDuration = 0.f;
};
