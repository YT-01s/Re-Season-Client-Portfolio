#include "BTTask_ResetThrowPattern.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_ResetThrowPattern::UBTTask_ResetThrowPattern()
{
    NodeName = TEXT("Reset Throw Pattern Flag");
}

EBTNodeResult::Type UBTTask_ResetThrowPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
        return EBTNodeResult::Failed;

    BB->SetValueAsBool("bShouldUseThrowPattern", false);
    UE_LOG(LogTemp, Warning, TEXT("[BTTask_ResetThrowPattern] bShouldUseThrowPattern reset to FALSE"));

    return EBTNodeResult::Succeeded;
}
