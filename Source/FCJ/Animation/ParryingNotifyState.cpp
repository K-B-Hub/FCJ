// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/ParryingNotifyState.h"
#include "PlayerCharacter/AttackCat.h"
#include "Components/SkeletalMeshComponent.h"

void UParryingNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// Get the AttackCat owner
	if (AAttackCat* AttackCat = Cast<AAttackCat>(MeshComp->GetOwner()))
	{
		AttackCat->StartParrying();
	}
}

void UParryingNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// Get the AttackCat owner
	if (AAttackCat* AttackCat = Cast<AAttackCat>(MeshComp->GetOwner()))
	{
		AttackCat->StopParrying();
	}
}