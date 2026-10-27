// Fill out your copyright notice in the Description page of Project Settings.


#include "CXCoinItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "CXPawn.h"
#include "CXPlayerState.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACXCoinItem::ACXCoinItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetNetUpdateFrequency(1.f);
	SetMinNetUpdateFrequency(1.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(SceneRoot);
	OverlapSphere->SetSphereRadius(100.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Overlap);
	OverlapSphere->SetGenerateOverlapEvents(true);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(SceneRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACXCoinItem::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ACXCoinItem::OnOverlapSphereBeginOverlap);
	}
}

void ACXCoinItem::OnOverlapSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
	{
		return;
	}

	ACXPawn* VehiclePawn = Cast<ACXPawn>(Other);
	if (IsValid(VehiclePawn) == false)
	{
		return;
	}

	ACXPlayerState* PlayerState = Cast<ACXPlayerState>(VehiclePawn->GetPlayerState());
	PlayerState->AddCoins(10);
	
	MulticastPickupEffects();

	Destroy();
}

void ACXCoinItem::MulticastPickupEffects_Implementation()
{
	UWorld* World = GetWorld();

	if (IsValid(PickupParticle))
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, PickupParticle, GetActorLocation(), GetActorRotation());
	}

	if (IsValid(PickupSound))
	{
		UGameplayStatics::SpawnSoundAtLocation(World, PickupSound, GetActorLocation(), GetActorRotation());
	}
}



