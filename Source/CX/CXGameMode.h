// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

UCLASS(MinimalAPI)
class ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

protected:
	virtual void BeginPlay() override;

protected:
	void StartWaitingCountdown();
	void HandleWatingTimerTick();
	void StartPlayingState();
	void HandlePlayingTimerTick();
	void FinishGame();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

private:
	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);

	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<class ACXGameStateBase> CXGameStateCached;

	FTimerHandle WaitingTimerHandle;
	float WaitingDuration = 10.0f;
	FTimerHandle PlayingTimerHandle;
	float PlayingDuration = 60.0f;
	bool bIsWaitingTimeStarted = false;
};



