// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Objects/AttachedPlatform.h"
#include "Actor/Objects/HoldingObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AAttachedPlatform::AAttachedPlatform()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root scene component
	USceneComponent* RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Create platform mesh
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(RootComponent);
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	PlatformMesh->SetIsReplicated(true);

	// Create attach trigger box
	AttachTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttachTriggerBox"));
	AttachTriggerBox->SetupAttachment(RootComponent);
	AttachTriggerBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	AttachTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttachTriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	AttachTriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AttachTriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	AttachTriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Overlap);
	AttachTriggerBox->SetGenerateOverlapEvents(true);

	// Default settings
	AttachOffset = FVector::ZeroVector;
	bDisablePhysicsOnAttach = true;
	AttachedObject = nullptr;

	// Enable replication
	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AAttachedPlatform::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	if (AttachTriggerBox)
	{
		AttachTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AAttachedPlatform::OnTriggerBoxBeginOverlap);
	}
}

void AAttachedPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAttachedPlatform, AttachedObject);
}

void AAttachedPlatform::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] OnTriggerBoxBeginOverlap - Actor: %s, Component: %s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"),
		OtherComp ? *OtherComp->GetName() : TEXT("NULL"));

	// Check if the overlapping actor is a HoldingObject
	AHoldingObject* HoldingObj = Cast<AHoldingObject>(OtherActor);
	if (!HoldingObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Not a HoldingObject"));
		return;
	}

	// Don't attach if already attached
	if (AttachedObject != nullptr)
	{
		AttachedObject = nullptr;
		//UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Already has an attached object: %s"), *AttachedObject->GetName());
		//return;
	}

	// Don't attach if the object is being held by a cat
	if (HoldingObj->IsBeingHeld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Object %s is being held by a cat"), *HoldingObj->GetName());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Calling ServerAttachObject for %s"), *HoldingObj->GetName());
	// Call server RPC to attach the object
	ServerAttachObject(HoldingObj);
}

void AAttachedPlatform::ServerAttachObject_Implementation(AHoldingObject* ObjectToAttach)
{
	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] ServerAttachObject_Implementation called for %s"),
		ObjectToAttach ? *ObjectToAttach->GetName() : TEXT("NULL"));

	if (!ObjectToAttach || AttachedObject != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] ServerAttachObject_Implementation - Invalid: ObjectToAttach=%s, AttachedObject=%s"),
			ObjectToAttach ? TEXT("Valid") : TEXT("NULL"),
			AttachedObject ? *AttachedObject->GetName() : TEXT("NULL"));
		return;
	}

	// Set replicated state
	AttachedObject = ObjectToAttach;

	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Calling MulticastAttachObject for %s"), *ObjectToAttach->GetName());
	// Broadcast to all clients
	MulticastAttachObject(ObjectToAttach);
}

void AAttachedPlatform::MulticastAttachObject_Implementation(AHoldingObject* ObjectToAttach)
{
	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] MulticastAttachObject_Implementation called for %s"),
		ObjectToAttach ? *ObjectToAttach->GetName() : TEXT("NULL"));

	if (!ObjectToAttach)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] MulticastAttachObject_Implementation - ObjectToAttach is NULL"));
		return;
	}

	AttachObjectToPlatform(ObjectToAttach);
}

void AAttachedPlatform::AttachObjectToPlatform(AHoldingObject* ObjectToAttach)
{
	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] AttachObjectToPlatform called for %s"),
		ObjectToAttach ? *ObjectToAttach->GetName() : TEXT("NULL"));

	if (!ObjectToAttach || !PlatformMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] AttachObjectToPlatform - Invalid: ObjectToAttach=%s, PlatformMesh=%s"),
			ObjectToAttach ? TEXT("Valid") : TEXT("NULL"),
			PlatformMesh ? TEXT("Valid") : TEXT("NULL"));
		return;
	}

	// If the object is being held, release it first
	if (ObjectToAttach->IsBeingHeld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Releasing held object before attaching"));
		ObjectToAttach->OnReleased();
	}

	// Disable physics if configured
	if (bDisablePhysicsOnAttach)
	{
		UStaticMeshComponent* ObjMesh = Cast<UStaticMeshComponent>(ObjectToAttach->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		if (ObjMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Disabling physics for %s"), *ObjectToAttach->GetName());
			ObjMesh->SetSimulatePhysics(false);
			ObjMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Could not find StaticMeshComponent on %s"), *ObjectToAttach->GetName());
		}
	}

	// Attach to platform mesh
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	ObjectToAttach->AttachToComponent(PlatformMesh, AttachRules);

	// Apply attach offset in platform's local space
	FVector CurrentLocation = ObjectToAttach->GetActorLocation();
	FVector OffsetInWorld = PlatformMesh->GetComponentTransform().TransformVector(AttachOffset);
	ObjectToAttach->SetActorLocation(CurrentLocation + OffsetInWorld);

	UE_LOG(LogTemp, Warning, TEXT("[AttachedPlatform] Object %s successfully attached to platform %s at location %s"),
		*ObjectToAttach->GetName(), *GetName(), *ObjectToAttach->GetActorLocation().ToString());
}

