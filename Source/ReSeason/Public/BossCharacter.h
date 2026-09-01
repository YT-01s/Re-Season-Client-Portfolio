// BossCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Damageable.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BossCharacter.generated.h"

// 보스 타입
UENUM(BlueprintType)
enum class EBossType : uint8
{
    Stone,
    Ice,
    Fire,
    FinalBoss
};

UCLASS()
class RESEASON_API ABossCharacter : public ACharacter, public IDamageable
{
    GENERATED_BODY()

    public:
    ABossCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    // 슬래시 이펙트
    UPROPERTY(EditAnywhere, Category = "VFX")
    UNiagaraSystem* SlashTrailEffect;

    UFUNCTION(BlueprintCallable)
    virtual void SpawnSlashEffect();

    // 공통 데미지 처리
    virtual void ApplyDamage_Implementation(float DamageAmount) override;

    // 이동 잠금 / 복원
    UFUNCTION(BlueprintCallable, Category = "Boss|Movement")
    void PreAttackLock(bool bLockMovement = true, bool bLockRotation = true);

    UFUNCTION(BlueprintCallable, Category = "Boss|Movement")
    void PostAttackUnlock();

    // 공통 상태
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
    bool bIsAttacking = false;

    // 공통 스탯
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Stats", meta = (ClampMin = "0"))
    float MaxHP = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats", meta = (ClampMin = "0"))
    float CurrentHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Info")
    EBossType BossType = EBossType::Stone;

    UFUNCTION(BlueprintCallable, Category = "Boss|Stats")
    float GetHPRatio() const { return (MaxHP > 0.f) ? CurrentHP / MaxHP : 0.f; }

    // 전투 공통 파라미터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat", meta = (ClampMin = "0"))
    float AttackDamage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat", meta = (ClampMin = "0"))
    float AttackRange = 200.f;          // 사정거리 중심

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat", meta = (ClampMin = "0"))
    float AttackRadius = 100.f;         // 원형 판정 반경

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Combat", meta = (ClampMin = "0"))
    float WeaponTraceRadius = 70.f;     // 궤적 트레이스 반경

    UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
    virtual float GetEffectiveAttackRange() const
    {
        return AttackRange + AttackRadius * 0.5f;
    }

    UPROPERTY(EditDefaultsOnly, Category = "Boss|AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // 공격/콤보 인터페이스 (파생에서 구현)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void PlayAttackMontage();        // 파생에서 구현

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void PlayComboAttackMontage();   // 파생에서 구현(기본 구현 포함)

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void DealDamage();               // AnimNotify에서 호출

    // 무기/트레이스 공통
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Weapon")
    UStaticMeshComponent* Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weapon")
    FVector WeaponScale = FVector(1.f, 1.f, 1.f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Weapon")
    FName WeaponAttachSocketName = TEXT("StoneEquip");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Weapon", meta = (ClampMin = "0"))
    float WeaponLength = 120.f;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FVector GetWeaponTipLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
    virtual void StartWeaponTrace();

    UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
    void StopWeaponTrace();

    // 외부에서 읽기 위한 게터 추가
    UFUNCTION(BlueprintPure, Category = "Boss|AI|Blackboard")
    FORCEINLINE FName GetBBKey_AttackCount() const { return BBKey_AttackCount; }

    UFUNCTION(BlueprintPure, Category = "Boss|AI|Blackboard")
    FORCEINLINE FName GetBBKey_CanCombo() const { return BBKey_CanCombo; }
protected:
    // 트레이스/콤보 내부 상태
    bool bWeaponActive = false;
    bool bCountedThisSwing = false;
    FVector LastWeaponTip;
    TSet<AActor*> HitActorsThisSwing;

    // 이동 원복용
    float SavedMaxWalkSpeed = 0.f;
    FRotator SavedRotationRate = FRotator::ZeroRotator;
    bool bSavedOrientRotationToMovement = true;

    // 콤보 공통
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Combo")
    int32 AttacksNeededForCombo = 3;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Combo")
    int32 NormalAttackCount = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Combo")
    float ComboCooldown = 5.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Combo")
    bool bComboOnCooldown = false;

    FTimerHandle ComboCooldownTimerHandle;

    void IncrementAttackCount();
    void ResetAttackCount();
    void StartComboCooldown();
    void OnComboCooldownFinished();
    void RecomputeAndPushCanCombo();

    // Blackboard Helper
    class UBlackboardComponent* GetBlackboard();
    void SetBBBool(FName Key, bool bValue);
    void SetBBInt(FName Key, int32 Value);

    // 보스 사망
    void OnDeath();

    // 공통 BB 키
    FName BBKey_AttackCount = TEXT("AttackCount");
    FName BBKey_CanCombo = TEXT("CanCombo");
};
