#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BossAIController.generated.h"

UCLASS()
class RESEASON_API ABossAIController : public AAIController
{
    GENERATED_BODY()

    public:
    ABossAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    FORCEINLINE UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }

protected:
    UPROPERTY(VisibleAnywhere, Category = "AI")
    class UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    class UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY()
    class UAISenseConfig_Sight* SightConfig;

    UFUNCTION()
    void HandlePerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

public:
    // °øÅë BB Å°
    static const FName KEY_TargetActor;
    static const FName KEY_HasTarget;
    static const FName KEY_LastKnownLocation;
};
