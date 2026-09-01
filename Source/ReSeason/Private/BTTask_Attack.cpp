#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossCharacter.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("ATTACK");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (ABossCharacter* Boss = Cast<ABossCharacter>(AICon->GetPawn()))
		{
			if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
			{
				if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))))
				{
					const float EffectiveRange = Boss->AttackRange + Boss->AttackRadius * 0.5f;
					const float DistSq = FVector::DistSquared(Boss->GetActorLocation(), Target->GetActorLocation());
					if (DistSq > FMath::Square(EffectiveRange))
					{
						return EBTNodeResult::Failed;
					}
				}
			}

			AICon->StopMovement();
			Boss->PlayAttackMontage();   // 카운트는 DealDamage(AnimNotify)에서 유효 히트 시 1회 증가
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
