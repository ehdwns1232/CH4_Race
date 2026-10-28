// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	None UMETA(DisplayName = "NONE"),
	Waiting UMETA(DisplayName = "WAITING"),
	Playing UMETA(DisplayName = "PLAYING"),
	Finished UMETA(DisplayName = "FINISHED")
};

class ACXPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangedCurrentGameState, EGameState, CurrentGameState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangedRemainingTime, int32, NewRemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnnouncedWinners, const TArray<ACXPlayerState*>&, FinalWinners);

UCLASS()
class CX_API ACXGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
	
	void SetGameState(EGameState NewGameState);
	void SetRemainingTime(int32 NewRemainingTime);
	void SetWinners(const TArray<ACXPlayerState*>& FinalWinners);
	int32 GetRemainingTime() const { return RemainingTime; }
	EGameState GetGameState() const { return CurrentGameState; }

	UFUNCTION()
	void OnRep_ChangedCurrentGameState();

	UFUNCTION()
	void OnRep_ChangedRemainingTime();

	UFUNCTION()
	void OnRep_AnnouncedWinners();

public:
	UPROPERTY(BlueprintAssignable)
	FOnChangedCurrentGameState OnChangedCurrentGameState;
	UPROPERTY(BlueprintAssignable)
	FOnChangedRemainingTime OnChangedRemainingTime;
	UPROPERTY(BlueprintAssignable)
	FOnAnnouncedWinners OnAnnouncedWinners;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ChangedCurrentGameState)
	EGameState CurrentGameState = EGameState::None;

	UPROPERTY(ReplicatedUsing = OnRep_ChangedRemainingTime)
	int32 RemainingTime = 0;

	UPROPERTY(ReplicatedUsing = OnRep_AnnouncedWinners)
	TArray<TObjectPtr<ACXPlayerState>> Winners;
};
