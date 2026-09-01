#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Notify_WeaponTraceState.generated.h"

UCLASS()
class RESEASON_API UNotify_WeaponTraceState : public UAnimNotifyState
{
    GENERATED_BODY()

    public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
