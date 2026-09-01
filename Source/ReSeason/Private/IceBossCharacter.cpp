#include "IceBossCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"

AIceBossCharacter::AIceBossCharacter()
{
    BossType = EBossType::Ice;

    WeaponAttachSocketName = TEXT("StoneEquip");
}

void AIceBossCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (Weapon)
    {
        Weapon->SetRelativeScale3D(WeaponScale);
        UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Weapon scale applied: %s"), *WeaponScale.ToString());
    }
}

void AIceBossCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AIceBossCharacter::ApplyDamage_Implementation(float DamageAmount)
{
    Super::ApplyDamage_Implementation(DamageAmount);

    if (CurrentHP <= 0.f) return;

    if (CurrentHP <= MaxHP * 0.5f && !bThrowPatternTriggered)
    {
        bThrowPatternTriggered = true;

        if (AAIController* AICon = Cast<AAIController>(GetController()))
        {
            if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
            {
                BB->SetValueAsBool(TEXT("bShouldUseThrowPattern"), true);
                UE_LOG(LogTemp, Warning, TEXT("[IceBoss] HP <= 50%% → bShouldUseThrowPattern = TRUE"));
            }
        }
    }
}

float AIceBossCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (CurrentHP <= MaxHP * 0.5f && !bThrowPatternTriggered)
    {
        bThrowPatternTriggered = true;

        if (AAIController* AICon = Cast<AAIController>(GetController()))
        {
            if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
            {
                BB->SetValueAsBool("bShouldUseThrowPattern", true);
                UE_LOG(LogTemp, Warning, TEXT("[IceBoss] HP <= 50%% → Trigger Throw Pattern"));
            }
        }
    }
    return ActualDamage;
}

void AIceBossCharacter::PlayAttackMontage()
{
    if (!GetMesh()) return;
    UAnimInstance* Anim = GetMesh()->GetAnimInstance();
    if (!Anim) return;

    // 왼손 / 오른손 교대
    UAnimMontage* Montage = bUseLeftAttack ? LeftAttackMontage : RightAttackMontage;
    if (!Montage)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] Missing Left/Right attack montage!"));
        return;
    }

    // 공격 시작 시 잠금
    bIsAttacking = true;
    bWeaponActive = true;                // 추가
    HitActorsThisSwing.Empty();          // 이번 스윙용 초기화
    LastWeaponTip = FVector::ZeroVector; // 이전 궤적 초기화

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsBool(TEXT("bIsAttacking"), true);
        }
    }

    // 이동 잠금 + 회전 잠금
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    const float PrevSpeed = MoveComp->MaxWalkSpeed;
    MoveComp->MaxWalkSpeed = 0.f;
    MoveComp->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = false;

    // 공격 몽타주 재생
    bCountedThisSwing = false;
    Anim->Montage_Play(Montage, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("[IceBoss] %s played (Left=%d)"),
        *Montage->GetName(), bUseLeftAttack ? 1 : 0);

    // 다음 공격엔 반대 손 사용
    bUseLeftAttack = !bUseLeftAttack;

    // 공격 종료 후 복구
    FOnMontageEnded EndDelegate;
    EndDelegate.BindLambda([this, PrevSpeed](UAnimMontage*, bool)
        {
            bIsAttacking = false;
            bWeaponActive = false; // 비활성화
            UCharacterMovementComponent* Move = GetCharacterMovement();
            Move->MaxWalkSpeed = PrevSpeed;
            Move->bOrientRotationToMovement = true;
            bUseControllerRotationYaw = true;

            if (AAIController* AICon = Cast<AAIController>(GetController()))
            {
                if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
                {
                    BB->SetValueAsBool(TEXT("bIsAttacking"), false);

                    if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))))
                    {
                        GetWorldTimerManager().SetTimerForNextTick([AICon, Target]()
                            {
                                AICon->MoveToActor(Target, 250.0f, true, true, true, nullptr, true);
                            });
                    }
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Attack montage ended — state reset."));
        });
    Anim->Montage_SetEndDelegate(EndDelegate, Montage);
}


void AIceBossCharacter::DealDamage()
{
    if (!bCountedThisSwing)
    {
        IncrementAttackCount();
        bCountedThisSwing = true;
        UE_LOG(LogTemp, Warning, TEXT("[IceBoss] DealDamage called"));
    }
}

void AIceBossCharacter::ThrowWeapon()
{
    if (!ThrownWeaponClass || !Weapon)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] ThrowWeapon aborted! Missing ThrownWeaponClass or Weapon."));
        return;
    }

    if (ThrowMontage && GetMesh() && GetMesh()->GetAnimInstance())
    {
        UAnimInstance* Anim = GetMesh()->GetAnimInstance();

        if (!Anim->Montage_IsPlaying(ThrowMontage))
        {
            Anim->Montage_Play(ThrowMontage, 1.0f);
            UE_LOG(LogTemp, Warning, TEXT("[IceBoss] ThrowMontage played."));
        }
    }

    FTimerHandle BackupTimer;
    GetWorldTimerManager().SetTimer(BackupTimer, this, &AIceBossCharacter::OnAnimNotify_Throw, ThrowDelayFallback, false);
}

// 투척
void AIceBossCharacter::OnAnimNotify_Throw()
{
    UE_LOG(LogTemp, Warning, TEXT("[AnimNotify_Throw] Fired!"));

    if (!ThrownWeaponClass || !Weapon)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] OnAnimNotify_Throw aborted! Missing class or weapon."));
        return;
    }

    // 스폰 위치 / 회전 계산
    FVector SpawnLoc;
    FRotator SpawnRot;

    if (Weapon->DoesSocketExist(TEXT("WeaponTip")))
    {
        const FTransform SocketTransform = Weapon->GetSocketTransform(TEXT("WeaponTip"), RTS_World);
        SpawnLoc = SocketTransform.GetLocation() + SocketTransform.GetRotation().GetForwardVector() * 20.f;
        SpawnRot = SocketTransform.GetRotation().Rotator();
    }
    else
    {
        SpawnLoc = Weapon->GetComponentLocation() + GetActorForwardVector() * 20.f;
        SpawnRot = GetActorRotation();
    }

    if (!FMath::IsFinite(SpawnLoc.X) || !FMath::IsFinite(SpawnLoc.Y) || !FMath::IsFinite(SpawnLoc.Z))
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] SpawnLoc invalid (NaN or INF)!"));
        SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
    }

    if (!FMath::IsFinite(SpawnRot.Pitch) || !FMath::IsFinite(SpawnRot.Yaw) || !FMath::IsFinite(SpawnRot.Roll))
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] SpawnRot invalid (NaN or INF)!"));
        SpawnRot = GetActorRotation();
    }

    // 타겟 방향 계산
    AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] Player not found."));
        return;
    }

    FVector Dir = (Player->GetActorLocation() + FVector(0, 0, 50.f)) - SpawnLoc;
    if (Dir.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Direction zero — skipping throw"));
        return;
    }
    Dir.Normalize();
    SpawnRot = Dir.Rotation();

    // 스폰 파라미터
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = GetInstigator();
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* ThrownWeapon = GetWorld()->SpawnActor<AActor>(ThrownWeaponClass, SpawnLoc, SpawnRot, Params);
    if (!ThrownWeapon)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] Failed to spawn thrown weapon."));
        return;
    }

    // 루트 컴포넌트 확인
    UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(ThrownWeapon->GetRootComponent());
    if (!RootComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] Thrown weapon root invalid."));
        return;
    }

    RootComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RootComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    RootComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    RootComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    RootComp->SetNotifyRigidBodyCollision(true);
    RootComp->SetGenerateOverlapEvents(false);
    RootComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));


    // ProjectileMovementComponent 생성
    UProjectileMovementComponent* ProjectileComp = NewObject<UProjectileMovementComponent>(ThrownWeapon);
    if (!ProjectileComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[IceBoss] Failed to create ProjectileMovementComponent."));
        return;
    }

    ProjectileComp->RegisterComponent();
    ProjectileComp->UpdatedComponent = RootComp;
    ProjectileComp->InitialSpeed = 3000.f;
    ProjectileComp->MaxSpeed = 3000.f;
    ProjectileComp->ProjectileGravityScale = 1.0f; // 1.0 = 기본 중력
    ProjectileComp->bRotationFollowsVelocity = true;
    ProjectileComp->Velocity = Dir * ProjectileComp->InitialSpeed;
    ProjectileComp->bShouldBounce = false;

    UE_LOG(LogTemp, Log, TEXT("[IceBoss] Projectile launched! Dir=%s | Speed=%.1f"),
        *Dir.ToString(), ProjectileComp->InitialSpeed);

    //콜리전 설정
    RootComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    RootComp->SetNotifyRigidBodyCollision(true);
    RootComp->IgnoreActorWhenMoving(this, true);

    // 충돌 감지
    RootComp->OnComponentHit.AddDynamic(this, &AIceBossCharacter::OnThrownWeaponHit);

    // 자동 소멸
    ThrownWeapon->SetLifeSpan(3.0f);
}

void AIceBossCharacter::PlayBackstepMontage()
{
    if (!BackstepMontage || !GetMesh()) return;

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(BackstepMontage, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("[IceBoss] Backstep montage played."));
    }
}

bool AIceBossCharacter::IsBackstepPlaying() const
{
    if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        return BackstepMontage && Anim->Montage_IsPlaying(BackstepMontage);
    }
    return false;
}

void AIceBossCharacter::OnThrownWeaponHit(
    UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    static bool bAlreadyHit = false;
    if (bAlreadyHit) return;
    bAlreadyHit = true;

    if (!OtherActor || OtherActor == this) return;

    UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Weapon Hit Something: %s"), *OtherActor->GetName());

    if (!OtherActor->ActorHasTag("Player"))
    {
        if (SpearImpactEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                SpearImpactEffect,
                Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation()
            );
            UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Impact effect spawned."));
        }
    }

    if (OtherActor->ActorHasTag("Player"))
    {
        const float Damage = 25.f;

        if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
        {
            IDamageable::Execute_ApplyDamage(OtherActor, Damage);
        }

        UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Hit Player — No Niagara Effect."));
    }

    AActor* ThisProjectile = Cast<AActor>(HitComp->GetOwner());
    if (IsValid(ThisProjectile) && ThisProjectile != this)
    {
        ThisProjectile->Destroy();
    }

    FTimerHandle ResetHandle;
    GetWorldTimerManager().SetTimer(ResetHandle, []() { bAlreadyHit = false; }, 0.1f, false);
}

void AIceBossCharacter::OnProjectileStopped(const FHitResult& ImpactResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Projectile stopped, destroying manually."));
    if (AActor* HitWeapon = ImpactResult.GetActor())
    {
        HitWeapon->Destroy();
    }
}

void AIceBossCharacter::SpawnSlashEffect()
{
    UE_LOG(LogTemp, Warning, TEXT("[IceBoss] SpawnSlashEffect() CALLED — Class: %s"), *GetClass()->GetName());

    if (!IceSlashTrail || !Weapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[IceBoss] IceSlashTrail or Weapon is NULL"));
        return;
    }

    UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
        IceSlashTrail,
        Weapon,
        TEXT("WeaponTip"),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTargetIncludingScale,
        true,
        true,
        ENCPoolMethod::AutoRelease
    );

    UE_LOG(LogTemp, Warning, TEXT("[IceBoss] Spawned Ice Trail Slash!"));
}