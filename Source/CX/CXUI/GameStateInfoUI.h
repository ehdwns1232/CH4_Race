// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStateInfoUI.generated.h"

class UTextBlock;
class ACXPlayerState;


UCLASS()
class CX_API UGameStateInfoUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnChangedCurrentGameState(EGameState NewGameState);
	UFUNCTION()
	void OnChangedRemainingTime(int32 NewRemainingTime);
	UFUNCTION()
	void OnShowWinners(const TArray<ACXPlayerState*>& FinalWinners);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentGameStateText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RemainingTimeText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WinnerNamesText;
};
