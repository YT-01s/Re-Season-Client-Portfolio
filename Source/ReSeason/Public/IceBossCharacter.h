#pragma once

#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "Animation/AnimMontage.h"
#include "IceBossCharacter.generated.h"

UCLASS()
class RESEASON_API AIceBossCharacter : public ABossCharacter
{
    GENERATED_BODY()

    public:
    AIceBossCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void PlayAttackMontage() override;
    virtual void DealDamage() override;
    virtual void ApplyDamage_Implementation(float DamageAmount) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    bool IsBackstepPlaying() const;
    UAnimMontage* GetBackstepMontage() const { return BackstepMontage; }

    void OnProjectileStopped(const FHitResult& ImpactResult);
    // 무기를 던지는 공격 (애니메이션 재생만) 
    UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
    void ThrowWeapon();

    // 실제 투척 로직 — AnimNotify에서 호출됨 
    UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
    void OnAnimNotify_Throw();

    // 백스텝 몽타주 재생 
    UFUNCTION(BlueprintCallable, Category = "Boss|Movement")
    void PlayBackstepMontage();

    // 던지기 애니메이션 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
    UAnimMontage* ThrowMontage;

    UFUNCTION()
    void OnThrownWeaponHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
protected:
    // 양손 공격 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice|Attack")
    UAnimMontage* LeftAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice|Attack")
    UAnimMontage* RightAttackMontage;

    // 공격 체인지
    bool bUseLeftAttack = true;

    // 백스텝 몽타주 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice|Skill")
    UAnimMontage* BackstepMontage;

    // 던지는 무기 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice|Skill")
    TSubclassOf<AActor> ThrownWeaponClass;

    // 던질 때 속도 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice|Skill")
    float ThrowSpeed = 4000.f;

    // 노티파이 실패 시 백업용 딜레이 (초 단위) 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice|Skill")
    float ThrowDelayFallback = 0.3f;

    // 한 번만 던지기 패턴 발동하도록 제어 
    bool bThrowPatternTriggered = false;

    virtual void SpawnSlashEffect() override;
    // 이펙트
    UPROPERTY(EditAnywhere, Category = "VFX")
    UNiagaraSystem* IceSlashTrail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    class UNiagaraSystem* SpearImpactEffect;
};