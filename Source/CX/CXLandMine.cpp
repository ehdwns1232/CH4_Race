#include "CXLandMine.h"
#include "Components/BoxComponent.h"
#include "CXPawn.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"


ACXLandMine::ACXLandMine()
{
 	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(SceneRoot);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BoxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Overlap);
	BoxCollision->SetGenerateOverlapEvents(true);
}

void ACXLandMine::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() == true)
	{
		BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ACXLandMine::OnBoxBeginOverlap);
	}
}

void ACXLandMine::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Other == GetOwner())
	{
		return;
	}

	ACXPawn* Pawn = Cast<ACXPawn>(Other);
	if (IsValid(Pawn) == false)
	{
		return;
	}

	Pawn->CTSBrokenAccelerator();

	MulticastPickupEffects();

	Destroy();
}

void ACXLandMine::MulticastPickupEffects_Implementation()
{
	UWorld* World = GetWorld();

	if (IsValid(OverlapParticle))
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, OverlapParticle, GetActorLocation(), GetActorRotation());
	}

	if (IsValid(OverlapSound))
	{
		UGameplayStatics::SpawnSoundAtLocation(World, OverlapSound, GetActorLocation(), GetActorRotation());
	}
}



