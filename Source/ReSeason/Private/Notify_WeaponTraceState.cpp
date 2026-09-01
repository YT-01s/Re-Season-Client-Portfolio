#include "Notify_WeaponTraceState.h"
#include "StoneBossCharacter.h"
#include "KatanaBase.h"

void UNotify_WeaponTraceState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// 보스
	if (ABossCharacter* Boss = Cast<ABossCharacter>(Owner))
	{
		Boss->StartWeaponTrace();
		UE_LOG(LogTemp, Warning, TEXT("[Notify] Weapon Trace BEGIN (Boss)"));
	}

	// 플레이어
	else if (AKatanaBase* Katana = Cast<AKatanaBase>(Owner))
	{
		Katana->StartWeaponTrace();
		UE_LOG(LogTemp, Warning, TEXT("[Notify] Weapon Trace BEGIN (Player)"));
	}
}

void UNotify_WeaponTraceState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// 보스
	if (ABossCharacter* Boss = Cast<ABossCharacter>(Owner))
	{
		Boss->StopWeaponTrace();
		UE_LOG(LogTemp, Warning, TEXT("[Notify] Weapon Trace END (Boss)"));
	}

	// 플레이어
	else if (AKatanaBase* Katana = Cast<AKatanaBase>(Owner))
	{
		Katana->StopWeaponTrace();
		UE_LOG(LogTemp, Warning, TEXT("[Notify] Weapon Trace END (Player)"));
	}
}
