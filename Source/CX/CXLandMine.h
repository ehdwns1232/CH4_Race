#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXLandMine.generated.h"

class UBoxComponent;

UCLASS()
class CX_API ACXLandMine : public AActor
{
	GENERATED_BODY()
	
public:	
	ACXLandMine();

protected:
	virtual void BeginPlay() override;

public:	
	UFUNCTION()
	virtual void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPickupEffects();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxCollision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particle")
	TObjectPtr<UParticleSystem> OverlapParticle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particle")
	TObjectPtr<USoundBase> OverlapSound;
};
