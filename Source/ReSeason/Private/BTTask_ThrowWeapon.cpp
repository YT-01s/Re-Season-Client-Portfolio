#include "BTTask_ThrowWeapon.h"
#include "AIController.h"
#include "IceBossCharacter.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h" 

UBTTask_ThrowWeapon::UBTTask_ThrowWeapon()
{
    NodeName = TEXT("Throw Weapon Pattern");
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_ThrowWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BTTask_ThrowWeapon] No AIController found"));
        return EBTNodeResult::Failed;
    }

    AIceBossCharacter* IceBoss = Cast<AIceBossCharacter>(AICon->GetPawn());
    if (!IceBoss)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BTTask_ThrowWeapon] Pawn is not IceBossCharacter"));
        return EBTNodeResult::Failed;
    }

    // 이동 정지
    AICon->StopMovement();
    if (UCharacterMovementComponent* MoveComp = IceBoss->GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
    }

    // 랜덤 던지기 횟수 설정
    RemainingThrows = FMath::RandRange(3, 5);
    UE_LOG(LogTemp, Warning, TEXT("[BTTask_ThrowWeapon] Start Throwing Pattern (%d times)"), RemainingThrows);

    // 첫 던지기 시작
    PerformThrow(&OwnerComp);
    return EBTNodeResult::InProgress;
}

void UBTTask_ThrowWeapon::PerformThrow(UBehaviorTreeComponent* OwnerComp)
{
    if (!OwnerComp) return;

    AAIController* AICon = OwnerComp->GetAIOwner();
    if (!AICon) return;

    AIceBossCharacter* IceBoss = Cast<AIceBossCharacter>(AICon->GetPawn());
    if (!IceBoss) return;

    IceBoss->ThrowWeapon();
    RemainingThrows--;

    if (RemainingThrows > 0)
    {
        // 다음 던지기 예약
        FTimerDelegate TimerDel;
        TimerDel.BindUObject(this, &UBTTask_ThrowWeapon::PerformThrow, OwnerComp);

        IceBoss->GetWorldTimerManager().SetTimer(
            ThrowTimerHandle,
            TimerDel,
            1.0f,   // 간격
            false
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BTTask_ThrowWeapon] Finished All Throws"));

        // 던지기 끝 → 이동 복귀
        if (UCharacterMovementComponent* MoveComp = IceBoss->GetCharacterMovement())
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }

        FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
    }
}
