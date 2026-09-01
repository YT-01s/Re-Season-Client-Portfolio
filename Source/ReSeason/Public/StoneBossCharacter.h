// StoneBossCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "EarthquakeZone.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "StoneBossCharacter.generated.h"

UCLASS()
class RESEASON_API AStoneBossCharacter : public ABossCharacter
{
    GENERATED_BODY()

    public:
    AStoneBossCharacter();

    // 式式 跦顫輿(Stone 瞪辨) 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    UAnimMontage* LeftAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    UAnimMontage* RightAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
    UAnimMontage* ComboAttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    bool bUseLeftAttack = true;

    // 式式 蝶鑒(Stone 瞪辨) 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
    UFUNCTION(BlueprintCallable, Category = "Boss|Skill")
    void SpawnEarthquake();

    UPROPERTY(EditAnywhere, Category = "Boss|Skill")
    TSubclassOf<class AEarthquakeZone> EarthquakeZoneClass;

    // 式式 奢問/巍爾 掘⑷ 式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式
    virtual void PlayAttackMontage() override;
    virtual void PlayComboAttackMontage() override;
    virtual void DealDamage() override;

    virtual void SpawnSlashEffect() override;
    // 檜めお
    UPROPERTY(EditAnywhere, Category = "VFX")
    UNiagaraSystem* StoneSlashTrail;

protected:
    virtual void BeginPlay() override;
};
