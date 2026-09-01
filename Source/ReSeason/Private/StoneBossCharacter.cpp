// StoneBossCharacter.cpp
#include "StoneBossCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

AStoneBossCharacter::AStoneBossCharacter()
{
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    AttackDamage = 25.f;       // ← 공통 변수 (ABossCharacter)
    AttackRange = 220.f;      // ← 공통 변수
    AttackRadius = 110.f;      // ← 공통 변수
}

void AStoneBossCharacter::BeginPlay()
{
    Super::BeginPlay();
    // 필요 시 추가 초기화
}

void AStoneBossCharacter::PlayAttackMontage()
{
    if (!GetMesh()) return;
    UAnimInstance* Anim = GetMesh()->GetAnimInstance();
    if (!Anim) return;

    // 양손 교대
    UAnimMontage* Montage = bUseLeftAttack ? LeftAttackMontage : RightAttackMontage;
    if (!Montage)
    {
        UE_LOG(LogTemp, Error, TEXT("[StoneBoss] Missing Left/Right attack montage!"));
        return;
    }

    // 이동/회전 잠금
    PreAttackLock(true, true);

    bCountedThisSwing = false;
    Anim->Montage_Play(Montage, 1.0f);

    // 끝나면 원복
    FOnMontageEnded End;
    End.BindLambda([this](UAnimMontage*, bool)
        {
            PostAttackUnlock();
        });
    Anim->Montage_SetEndDelegate(End, Montage);

    // 다음 손 전환
    bUseLeftAttack = !bUseLeftAttack;

    UE_LOG(LogTemp, Warning, TEXT("[StoneBoss] %s played (Left=%d)"),
        *Montage->GetName(), bUseLeftAttack ? 0 : 1);
}

void AStoneBossCharacter::PlayComboAttackMontage()
{
    if (ComboAttackMontage && GetMesh() && GetMesh()->GetAnimInstance())
    {
        UAnimInstance* Anim = GetMesh()->GetAnimInstance();

        PreAttackLock(true, true);
        Anim->Montage_Play(ComboAttackMontage, 1.0f);

        FOnMontageEnded End;
        End.BindLambda([this](UAnimMontage*, bool)
            {
                PostAttackUnlock();
            });
        Anim->Montage_SetEndDelegate(End, ComboAttackMontage);
    }

    // 공통 콤보 처리(카운트 리셋/쿨다운 시작)
    Super::PlayComboAttackMontage();
}

void AStoneBossCharacter::DealDamage()
{
    // 공통의 1회 카운트 증가 처리 유지
    Super::DealDamage();
}

void AStoneBossCharacter::SpawnEarthquake()
{
    if (!EarthquakeZoneClass) return;

    FActorSpawnParameters P;
    P.Owner = this;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 보스의 중심 위치
    FVector SpawnLoc = GetActorLocation();

    // 보스 캡슐 하단(Z)으로 보정
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        SpawnLoc.Z -= HalfHeight; // 캡슐 중심에서 절반 아래 = 발 위치
    }

    FHitResult Hit;
    FVector Start = SpawnLoc + FVector(0, 0, 100.f);
    FVector End = SpawnLoc - FVector(0, 0, 500.f);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
    {
        SpawnLoc.Z = Hit.Location.Z; // 지면 높이에 맞춤
    }

    // 스폰 회전
    const FRotator SpawnRot = FRotator::ZeroRotator;

    // 장판 생성
    GetWorld()->SpawnActor<AEarthquakeZone>(EarthquakeZoneClass, SpawnLoc, SpawnRot, P);

}

void AStoneBossCharacter::SpawnSlashEffect()
{
    UE_LOG(LogTemp, Warning, TEXT("[StoneBoss] SpawnSlashEffect() CALLED — Class: %s"), *GetClass()->GetName());

    if (!StoneSlashTrail || !Weapon) return;

    UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
        StoneSlashTrail,
        Weapon,
        TEXT("WeaponTip"), // 소켓 이름
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTargetIncludingScale,
        true,
        true,
        ENCPoolMethod::AutoRelease
    );

    UE_LOG(LogTemp, Warning, TEXT("[StoneBoss] Spawned Stone Trail Slash!"));
}