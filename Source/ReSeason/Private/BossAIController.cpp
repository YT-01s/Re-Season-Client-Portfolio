#include "BossAIController.h"
#include "BossCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

const FName ABossAIController::KEY_TargetActor(TEXT("TargetActor"));
const FName ABossAIController::KEY_HasTarget(TEXT("HasTarget"));
const FName ABossAIController::KEY_LastKnownLocation(TEXT("LastKnownLocation"));

ABossAIController::ABossAIController()
{
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    SightConfig->SightRadius = 1500.f;
    SightConfig->LoseSightRadius = 2000.f;
    SightConfig->PeripheralVisionAngleDegrees = 120.f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABossAIController::HandlePerceptionUpdated);

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.2f;  // 0.2초마다 Distance 갱신
}

void ABossAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (ABossCharacter* Boss = Cast<ABossCharacter>(InPawn))
    {
        if (Boss->BehaviorTreeAsset)
        {
            UseBlackboard(Boss->BehaviorTreeAsset->BlackboardAsset, BlackboardComp);
            RunBehaviorTree(Boss->BehaviorTreeAsset);
            BehaviorComp->StartTree(*Boss->BehaviorTreeAsset);

            BlackboardComp->SetValueAsInt(TEXT("bCanAttack"), 1);
            BlackboardComp->SetValueAsInt(TEXT("bIsAttacking"), 0);
            BlackboardComp->SetValueAsFloat(TEXT("Distance"), 0.f);
        }
    }
}

void ABossAIController::OnUnPossess()
{
    Super::OnUnPossess();
    if (BehaviorComp && BehaviorComp->IsRunning())
    {
        BehaviorComp->StopTree(EBTStopMode::Safe);
    }
}

void ABossAIController::HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!BlackboardComp) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        BlackboardComp->SetValueAsObject(KEY_TargetActor, Actor);
        BlackboardComp->SetValueAsBool(KEY_HasTarget, true);
        //SetFocus(Actor);
    }
    else
    {
        BlackboardComp->SetValueAsBool(KEY_HasTarget, false);
        BlackboardComp->SetValueAsVector(KEY_LastKnownLocation, Stimulus.StimulusLocation);
        //ClearFocus(EAIFocusPriority::Gameplay);
    }
}

void ABossAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!BlackboardComp) return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(KEY_TargetActor));
    if (!Target) return;

    //  거리 계산 및 블랙보드 갱신
    const float Distance = FVector::Dist(Target->GetActorLocation(), ControlledPawn->GetActorLocation());
    BlackboardComp->SetValueAsFloat(TEXT("Distance"), Distance);
}