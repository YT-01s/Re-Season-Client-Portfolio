#include "BTTask_Backstep.h"
#include "IceBossCharacter.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"

UBTTask_Backstep::UBTTask_Backstep()
{
    NodeName = TEXT("Backstep");
    bNotifyTick = true; // TickTask 활성화
}

EBTNodeResult::Type UBTTask_Backstep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    AIceBossCharacter* IceBoss = Cast<AIceBossCharacter>(AICon->GetPawn());
    if (!IceBoss || !IceBoss->GetMesh()) return EBTNodeResult::Failed;

    // 백스텝 몽타주가 존재하지 않으면 즉시 실패
    if (!IceBoss->GetBackstepMontage())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BTTask_Backstep] BackstepMontage is NULL!"));
        return EBTNodeResult::Failed;
    }

    // 백스텝 몽타주 재생
    UAnimInstance* Anim = IceBoss->GetMesh()->GetAnimInstance();
    if (Anim)
    {
        IceBoss->PlayBackstepMontage();
        UE_LOG(LogTemp, Log, TEXT("[BTTask_Backstep] Backstep montage started."));

        // 루트모션이 없을 경우 대비해 약간의 Launch 추가
        FVector BackDir = -IceBoss->GetActorForwardVector();
        IceBoss->LaunchCharacter(BackDir * 150.f, true, false);
    }

    //  Tick에서 완료 감시 → InProgress로 리턴
    return EBTNodeResult::InProgress;
}

void UBTTask_Backstep::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return;

    AIceBossCharacter* IceBoss = Cast<AIceBossCharacter>(AICon->GetPawn());
    if (!IceBoss || !IceBoss->GetMesh()) return;

    UAnimInstance* Anim = IceBoss->GetMesh()->GetAnimInstance();
    if (!Anim || !IceBoss->GetBackstepMontage()) return;

    // 루트모션 기반이므로 애니메이션 종료까지 기다림
    if (!Anim->Montage_IsPlaying(IceBoss->GetBackstepMontage()))
    {
        UE_LOG(LogTemp, Log, TEXT("[BTTask_Backstep] Backstep montage finished."));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_Backstep::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    bIsMontagePlaying = false;
    MontageDuration = 0.f;
}
