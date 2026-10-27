// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXSpeedBuffItem.generated.h"

class USceneComponent;
class UPrimitiveComponent;
class USphereComponent;
class USoundBase;

UCLASS()
class CX_API ACXSpeedBuffItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACXSpeedBuffItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPickupEffects();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff")
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff")
	TObjectPtr<USphereComponent> OverlapSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Buff")
	TObjectPtr<UStaticMeshComponent> ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	TObjectPtr<UParticleSystem> PickupParticle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	TObjectPtr<USoundBase> PickupSound;
};
