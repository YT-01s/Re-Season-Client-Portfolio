#include "EarthquakeZone.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Damageable.h"

AEarthquakeZone::AEarthquakeZone()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    DamageZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageZone"));
    DamageZone->SetupAttachment(Root);
    DamageZone->InitBoxExtent(BoxExtent);
    DamageZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DamageZone->SetCollisionObjectType(ECC_WorldDynamic);
    DamageZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    EarthquakeVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EarthquakeVFX"));
    EarthquakeVFX->SetupAttachment(Root);
}

void AEarthquakeZone::BeginPlay()
{
    Super::BeginPlay();

    if (EarthquakeVFX)
    {
        EarthquakeVFX->Activate(true);
        UE_LOG(LogTemp, Warning, TEXT("[EQ] EarthquakeVFX Activated!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[EQ] EarthquakeVFX is NULL!"));
    }

    // C++ 파라미터 -> 실제 콜라이더 크기 동기화
    //DamageZone->SetBoxExtent(BoxExtent);
    DamageZone->SetGenerateOverlapEvents(true);
    DamageZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 스폰 프레임에 겹침 갱신
    DamageZone->UpdateOverlaps();

    // 애니 타이밍을 위해 지연 가능
    if (ActivationDelay > 0.f)
    {
        GetWorldTimerManager().SetTimer(Timer_Impact, this, &AEarthquakeZone::DoOneShotDamage, ActivationDelay, false);
    }
    else
    {
        DoOneShotDamage();
    }

    // VFX 유지 후 자동 파괴
    SetLifeSpan(Lifetime);

    // 시각 확인
    //DrawDebugBox(GetWorld(), DamageZone->GetComponentLocation(),
        //DamageZone->GetScaledBoxExtent(), FColor::Red, false, 1.5f);
}

void AEarthquakeZone::DoOneShotDamage()
{
    if (bHasAppliedDamage) return;
    bHasAppliedDamage = true;

    // ActivationDelay 때문에 실제 충돌 갱신 필요
    DamageZone->UpdateOverlaps();

    TArray<AActor*> Overlapping;
    DamageZone->GetOverlappingActors(Overlapping, ACharacter::StaticClass());

    for (AActor* A : Overlapping)
    {
        if (!A || A == this || A == GetOwner()) continue;

        ACharacter* Player = Cast<ACharacter>(A);
        if (!Player) continue;

        UCharacterMovementComponent* Move = Player->GetCharacterMovement();
        const bool bInAir = Move && Move->IsFalling();
        const bool bOffGround = Move && !Move->IsMovingOnGround();
        const bool bPressedJump = Player->JumpCurrentCount > 0;

        if (bInAir || bOffGround || bPressedJump)
        {
            //UE_LOG(LogTemp, Warning, TEXT("[EQ] 점프로 회피! %s"), *Player->GetName());
        }
        else
        {
            // 인터페이스 기반 데미지 처리
            if (Player->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
            {
                IDamageable::Execute_ApplyDamage(Player, Damage);
                UE_LOG(LogTemp, Warning, TEXT("지진 데미지 → %f damage"), Damage);
            }
            else
            {
                // fallback: 일반 AActor 처리
                UGameplayStatics::ApplyDamage(Player, Damage, nullptr, this, nullptr);
                UE_LOG(LogTemp, Warning, TEXT("[EQ] %s hit by Earthquake (fallback) → %f damage"),
                    *Player->GetName(), Damage);
            }
        }
    }

    DamageZone->SetGenerateOverlapEvents(false);
    DamageZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

