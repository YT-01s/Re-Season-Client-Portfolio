// BossCharacter.cpp
#include "BossCharacter.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "MyPlayer.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ABossCharacter::ABossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // 공통 무기 컴포넌트
    Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
    Weapon->SetupAttachment(GetMesh());
    Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABossCharacter::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (Weapon)
    {
        Weapon->SetRelativeScale3D(WeaponScale);
    }
}

void ABossCharacter::BeginPlay()
{
    Super::BeginPlay();

    // HP 초기화
    CurrentHP = FMath::Clamp(MaxHP, 0.f, MaxHP);

    if (Weapon && GetMesh())
    {
        Weapon->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            WeaponAttachSocketName
        );
    }


    // BB 초기화 (딜레이 한 틱)
    FTimerHandle InitBBHandle;
    GetWorldTimerManager().SetTimer(
        InitBBHandle,
        [this]()
        {
            if (UBlackboardComponent* BB = GetBlackboard())
            {
                SetBBInt(BBKey_AttackCount, 0);
                SetBBBool(BBKey_CanCombo, false);
            }
        },
        0.1f, false);
}

void ABossCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bWeaponActive) return;

    const FVector CurrentTip = GetWeaponTipLocation();
    if (LastWeaponTip.IsNearlyZero())
    {
        LastWeaponTip = CurrentTip;
        return;
    }

    // 궤적 스윕
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BossWeaponSweep), false);
    Params.AddIgnoredActor(this);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    const FCollisionShape Sphere = FCollisionShape::MakeSphere(WeaponTraceRadius);

    const bool bAnyHit = GetWorld()->SweepMultiByObjectType(
        Hits, LastWeaponTip, CurrentTip, FQuat::Identity, ObjParams, Sphere, Params);

    if (bAnyHit)
    {
        for (const FHitResult& Hit : Hits)
        {
            AActor* HitActor = Hit.GetActor();
            if (!HitActor || HitActor == this) continue;
            if (HitActorsThisSwing.Contains(HitActor)) continue;

            HitActorsThisSwing.Add(HitActor);

            // 데미지 처리
            if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
            {
                IDamageable::Execute_ApplyDamage(HitActor, AttackDamage);
            }
            else
            {
                UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, nullptr);
            }
        }
    }

    LastWeaponTip = CurrentTip;
}

void ABossCharacter::OnDeath()
{
    AMyPlayer* Player = Cast<AMyPlayer>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Player && Player->PurificationInstance)
    {
        switch (BossType)
        {
        case EBossType::Stone:
            Player->PurificationInstance->ApplyPurificationAction(EPurificationAction::Boss_StoneDefeated);
            break;
        case EBossType::Fire:
            Player->PurificationInstance->ApplyPurificationAction(EPurificationAction::Boss_FireDefeated);
            break;
        case EBossType::Ice:
            Player->PurificationInstance->ApplyPurificationAction(EPurificationAction::Boss_IceDefeated);
            break;
        case EBossType::FinalBoss:
            Player->PurificationInstance->ApplyPurificationAction(EPurificationAction::Boss_FinalDefeated);
            break;
        }

        UE_LOG(LogTemp, Warning, TEXT("[Purification] Boss defeated — gauge increased!"));
    }

    Destroy();
}


void ABossCharacter::ApplyDamage_Implementation(float DamageAmount)
{

    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);
    UE_LOG(LogTemp, Warning, TEXT("[BossCharacter] Took Damage: %.1f | HP: %.1f / %.1f"), DamageAmount, CurrentHP, MaxHP);

    if (CurrentHP <= 0.f)
    {
        OnDeath();
    }
}

// 이동 잠금/복원
void ABossCharacter::PreAttackLock(bool bLockMovement, bool bLockRotation)
{
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UPathFollowingComponent* PFC = AIC->GetPathFollowingComponent())
        {
            const FAIRequestID ReqId = PFC->GetCurrentRequestId();
            if (ReqId.IsValid() && PFC->GetStatus() == EPathFollowingStatus::Moving)
            {
                if (bLockMovement)
                {
                    PFC->PauseMove(ReqId, EPathFollowingVelocityMode::Keep);
                }
            }
        }
    }

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        SavedMaxWalkSpeed = Move->MaxWalkSpeed;
        SavedRotationRate = Move->RotationRate;
        bSavedOrientRotationToMovement = Move->bOrientRotationToMovement;

        if (bLockMovement)
        {
            Move->StopMovementImmediately();
            Move->DisableMovement();
            //Move->MaxWalkSpeed = 0.f;
            Move->BrakingFrictionFactor = 10.f;
        }
        if (bLockRotation)
        {
            bUseControllerRotationYaw = false;
            Move->bOrientRotationToMovement = false;
        }
    }

    bIsAttacking = true;
}

void ABossCharacter::PostAttackUnlock()
{
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
        Move->MaxWalkSpeed = SavedMaxWalkSpeed;
        Move->BrakingFrictionFactor = 2.f;
        Move->bOrientRotationToMovement = bSavedOrientRotationToMovement;
        Move->RotationRate = SavedRotationRate;
    }
    bUseControllerRotationYaw = true;
    bIsAttacking = false;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UPathFollowingComponent* PFC = AIC->GetPathFollowingComponent())
        {
            const FAIRequestID ReqId = PFC->GetCurrentRequestId();
            if (ReqId.IsValid())
            {
                PFC->ResumeMove(ReqId);
            }
        }
    }
}

// 무기 관련
FVector ABossCharacter::GetWeaponTipLocation() const
{
    if (Weapon && Weapon->DoesSocketExist(TEXT("WeaponTip")))
        return Weapon->GetSocketLocation(TEXT("WeaponTip"));

    if (Weapon)
        return Weapon->GetComponentLocation();

    return GetActorLocation();
}

void ABossCharacter::StartWeaponTrace()
{
    bWeaponActive = true;
    bCountedThisSwing = false;
    HitActorsThisSwing.Empty();
    LastWeaponTip = GetWeaponTipLocation();

    UE_LOG(LogTemp, Warning, TEXT("[WeaponTrace] BEGIN — Tip at %s"), *LastWeaponTip.ToString());
    SpawnSlashEffect();
}

void ABossCharacter::StopWeaponTrace()
{
    bWeaponActive = false;
    LastWeaponTip = FVector::ZeroVector;
    HitActorsThisSwing.Empty();

    UE_LOG(LogTemp, Warning, TEXT("[WeaponTrace] END"));

    if (Weapon)
    {
        for (USceneComponent* Child : Weapon->GetAttachChildren())
        {
            if (UNiagaraComponent* NC = Cast<UNiagaraComponent>(Child))
            {
                NC->Deactivate();
            }
        }
    }
}

// 콤보/BB
UBlackboardComponent* ABossCharacter::GetBlackboard()
{
    if (AAIController* AIC = Cast<AAIController>(GetController()))
        return AIC->GetBlackboardComponent();
    return nullptr;
}

void ABossCharacter::SetBBBool(FName Key, bool bValue)
{
    if (UBlackboardComponent* BB = GetBlackboard())
        BB->SetValueAsBool(Key, bValue);
}

void ABossCharacter::SetBBInt(FName Key, int32 Value)
{
    if (UBlackboardComponent* BB = GetBlackboard())
        BB->SetValueAsInt(Key, Value);
}

void ABossCharacter::IncrementAttackCount()
{
    NormalAttackCount = FMath::Max(0, NormalAttackCount + 1);
    SetBBInt(BBKey_AttackCount, NormalAttackCount);
    RecomputeAndPushCanCombo();
}

void ABossCharacter::ResetAttackCount()
{
    NormalAttackCount = 0;
    SetBBInt(BBKey_AttackCount, 0);
}

void ABossCharacter::StartComboCooldown()
{
    GetWorldTimerManager().SetTimer(
        ComboCooldownTimerHandle, this,
        &ABossCharacter::OnComboCooldownFinished,
        ComboCooldown, false);
}

void ABossCharacter::OnComboCooldownFinished()
{
    bComboOnCooldown = false;
    RecomputeAndPushCanCombo();
}

void ABossCharacter::RecomputeAndPushCanCombo()
{
    const bool bCan = (!bComboOnCooldown) && (NormalAttackCount >= AttacksNeededForCombo);
    SetBBBool(BBKey_CanCombo, bCan);
}

// 기본 구현
void ABossCharacter::PlayAttackMontage() {}
void ABossCharacter::PlayComboAttackMontage()
{
    ResetAttackCount();
    bComboOnCooldown = true;
    SetBBBool(BBKey_CanCombo, false);
    StartComboCooldown();
}
void ABossCharacter::DealDamage()
{
    UE_LOG(LogTemp, Warning, TEXT("[DealDamage] called. bCountedThisSwing=%d"),
        bCountedThisSwing ? 1 : 0);

    if (!bCountedThisSwing)
    {
        IncrementAttackCount();
        bCountedThisSwing = true;
        UE_LOG(LogTemp, Warning, TEXT("[DealDamage] ++count -> %d"), NormalAttackCount);
    }
}

void ABossCharacter::SpawnSlashEffect()
{

}
