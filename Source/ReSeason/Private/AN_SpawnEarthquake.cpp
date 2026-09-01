#include "AN_SpawnEarthquake.h"
#include "StoneBossCharacter.h"

void UAN_SpawnEarthquake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    UE_LOG(LogTemp, Warning, TEXT(" AN_SpawnEarthquake Triggered!"));

    if (AStoneBossCharacter* Boss = Cast<AStoneBossCharacter>(MeshComp->GetOwner()))
    {
        UE_LOG(LogTemp, Warning, TEXT(" Boss Found! Spawning Earthquake..."));
        Boss->SpawnEarthquake();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Boss not found! Notify failed."));
    }
}