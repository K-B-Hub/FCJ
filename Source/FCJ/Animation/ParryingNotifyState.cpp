// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/ParryingNotifyState.h"
#include "PlayerCharacter/AttackCat.h"
#include "PlayerCharacter/HybridCat.h"
#include "Components/SkeletalMeshComponent.h"

void UParryingNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	// AttackCat support (backward compatibility)
	if (AAttackCat* AttackCat = Cast<AAttackCat>(Owner))
	{
		AttackCat->StartParrying();

		// 패링 시작 시 주변 물체 밀치기
		AttackCat->PushNearbyObjects();
	}
	// HybridCat support
	else if (AHybridCat* HybridCat = Cast<AHybridCat>(Owner))
	{
		HybridCat->StartParrying();

		// 패링 시작 시 주변 물체 밀치기
		HybridCat->PushNearbyObjects();
	}
}

void UParryingNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	// AttackCat support (backward compatibility)
	if (AAttackCat* AttackCat = Cast<AAttackCat>(Owner))
	{
		AttackCat->StopParrying();
	}
	// HybridCat support
	else if (AHybridCat* HybridCat = Cast<AHybridCat>(Owner))
	{
		HybridCat->StopParrying();
	}
}