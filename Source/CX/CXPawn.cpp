// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "CXPlayerState.h"
#include "CXGameStateBase.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "CXLandMine.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	bReplicates = true;
	bCanAccelerate = true;
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

}

void ACXPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (ACXGameStateBase* CXGameStateBase = World->GetGameState<ACXGameStateBase>())
		{
			CXGameStateBase->OnChangedCurrentGameState.AddDynamic(this, &ACXPawn::SetControlOption);
		}
	}
}

void ACXPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(BrokenAcceleratorTimer);

	Super::EndPlay(EndPlayReason);
}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::InputAttack);

		EnhancedInputComponent->BindAction(LandMineAction, ETriggerEvent::Started, this, &ACXPawn::InputLandMine);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	// add some angular damping if the vehicle is in midair
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// realign the camera yaw to face front
	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);

	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}

float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);
	if (ActualDamage <= 0.f || DamageCauser == this || HasAuthority() == false || Attacker == nullptr)
	{
		return 0.f;
	}

	ACXPlayerState* VictimPlayerState = GetPlayerState<ACXPlayerState>();
	ACXPlayerState* AttackerPlayerState = Attacker->GetPlayerState<ACXPlayerState>();

	const int32 TrasferCoinAmount = (int32)ActualDamage;
	if (VictimPlayerState->TryRemoveCoins(TrasferCoinAmount))
	{
		AttackerPlayerState->AddCoins(TrasferCoinAmount);
	}
	
	return ActualDamage;
}

void ACXPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bCanAccelerate);
}

void ACXPawn::MulticastHitEffect_Implementation(const FVector& TraceStart, const FVector& TraceEnd, bool bHitVehicle, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	UWorld* World = GetWorld();
	DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, false, 1.0f, 0, 3.0f);
	if (!bHitVehicle)
	{
		return;
	}

	UGameplayStatics::SpawnEmitterAtLocation(World, HitParticle, ImpactPoint, ImpactNormal.Rotation());
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
	if (bCanAccelerate == false || bCanControl == false)
	{
		ChaosVehicleMovement->SetSteeringInput(0.0f);
		return;
	}

	// get the input magnitude for steering
	float SteeringValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
	if (bCanAccelerate == false || bCanControl == false)
	{
		ChaosVehicleMovement->SetThrottleInput(0.0f);
		return;
	}

	// get the input magnitude for the throttle
	float ThrottleValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	// get the input magnitude for the brakes
	float BreakValue = Value.Get<float>();

	// add the input
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void ACXPawn::StartBrake(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopBrake(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	// call the Blueprint hook for the break lights
	BrakeLights(false);

	// reset brake input to zero
	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(true);

	// call the Blueprint hook for the break lights
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	// add the input
	ChaosVehicleMovement->SetHandbrakeInput(false);

	// call the Blueprint hook for the break lights
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void ACXPawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

void ACXPawn::InputAttack(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	CTSAttack();
}

void ACXPawn::InputLandMine(const FInputActionValue& Value)
{
	if (bCanControl == false)
	{
		return;
	}

	FVector BackDir = -GetActorForwardVector() * 250.0f;
	FVector SpawnLocation = GetActorLocation() + BackDir;
	SpawnLocation.Z = 0.0f;
	FRotator SpawnRotation = GetActorRotation();
	SpawnRotation.Roll = 0.0f;
	SpawnRotation.Pitch = 0.0f;

	CTSLandMine(SpawnLocation, SpawnRotation);
}

void ACXPawn::SetControlOption(EGameState NewGameState)
{
	if (NewGameState == EGameState::Playing)
	{
		bCanControl = true;
		ChaosVehicleMovement->SetHandbrakeInput(false);
	}
	else
	{
		bCanControl = false;
		ChaosVehicleMovement->SetHandbrakeInput(true);
	}
}

void ACXPawn::CTSBrokenAccelerator_Implementation()
{
	if (bCanAccelerate == false)
	{
		return;
	}

	bCanAccelerate = false;
	ChaosVehicleMovement->StopMovementImmediately();
	ChaosVehicleMovement->SetThrottleInput(0.0f);
	
	GetWorldTimerManager().SetTimer(BrokenAcceleratorTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			bCanAccelerate = true;
		}), BrokenDuration, false);
}

void ACXPawn::CTSLandMine_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	ACXLandMine* SpawnedLandMine = GetWorld()->SpawnActorDeferred<ACXLandMine>(LandMineClass, FTransform(SpawnRotation, SpawnLocation), this);
	if (SpawnedLandMine)
	{
		UGameplayStatics::FinishSpawningActor(SpawnedLandMine, FTransform(SpawnRotation, SpawnLocation));
	}
}

void ACXPawn::CTSAttack_Implementation()
{
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 200.0f;
	const FVector End = Start + GetActorForwardVector() * 300.0f;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, Params);
	bool bHitVehicle = false;
	if (bHit)
	{
		AActor* Actor = HitResult.GetActor();
		UE_LOG(LogTemplateVehicle, Warning, TEXT("Attack trace hit : %s"), *GetNameSafe(Actor));

		if (ACXPawn* CXPawn = Cast<ACXPawn>(Actor))
		{
			bHitVehicle = true;
			FVector HitFromDirection = (HitResult.ImpactPoint - Start).GetSafeNormal();
			UGameplayStatics::ApplyPointDamage(CXPawn, 1.0f, HitFromDirection, HitResult, GetController(), this, nullptr);
			FString BoneName = HitResult.BoneName.ToString();
			if (BoneName.Contains(TEXT("Wheel"), ESearchCase::IgnoreCase))
			{
				if (ACXPlayerState* CXPlayerState = CXPawn->GetPlayerState<ACXPlayerState>())
				{
					CXPlayerState->ApplyPenalty();
				}
			}
		}
	}

	MulticastHitEffect(Start, End, bHitVehicle, HitResult.ImpactPoint, HitResult.ImpactNormal);
}

#undef LOCTEXT_NAMESPACE