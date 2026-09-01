#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EarthquakeZone.generated.h"

UCLASS()
class RESEASON_API AEarthquakeZone : public AActor
{
    GENERATED_BODY()

    public:
    AEarthquakeZone();

protected:
    virtual void BeginPlay() override;

    // --- Components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* DamageZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* EarthquakeVFX;

    // --- Tunables ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake")
    float Damage = 30.f;

    // 보스 애니메이션 타이밍에 맞추고 싶을 때 사용 (0이면 즉시 판정)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake")
    float ActivationDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake")
    float Lifetime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Earthquake")
    FVector BoxExtent = FVector(400.f, 400.f, 200.f);


private:
    void DoOneShotDamage();           // ← 스폰 시점 “1회만” 판정
    bool bHasAppliedDamage = false;   // 중복 방지
    FTimerHandle Timer_Impact;
};