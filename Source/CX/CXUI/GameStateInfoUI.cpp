// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStateInfoUI.h"
#include "CXGameStateBase.h"
#include "CXPlayerState.h"
#include "Components/TextBlock.h"

void UGameStateInfoUI::NativeConstruct()
{
	Super::NativeConstruct();

	ACXGameStateBase* CXGameStateBase = GetWorld()->GetGameState<ACXGameStateBase>();
	if (CXGameStateBase == nullptr)
	{
		return;
	}

	if (IsValid(WinnerNamesText) == false)
	{
		return;
	}

	WinnerNamesText->SetVisibility(ESlateVisibility::Collapsed);
	if (CXGameStateBase->OnChangedCurrentGameState.IsAlreadyBound(this, &UGameStateInfoUI::OnChangedCurrentGameState) == false)
	{
		CXGameStateBase->OnChangedCurrentGameState.AddDynamic(this, &UGameStateInfoUI::OnChangedCurrentGameState);
	}
	if (CXGameStateBase->OnChangedRemainingTime.IsAlreadyBound(this, &UGameStateInfoUI::OnChangedRemainingTime) == false)
	{
		CXGameStateBase->OnChangedRemainingTime.AddDynamic(this, &UGameStateInfoUI::OnChangedRemainingTime);
	}
	if (CXGameStateBase->OnAnnouncedWinners.IsAlreadyBound(this, &UGameStateInfoUI::OnShowWinners) == false)
	{
		CXGameStateBase->OnAnnouncedWinners.AddDynamic(this, &UGameStateInfoUI::OnShowWinners);
	}
}

void UGameStateInfoUI::NativeDestruct()
{
	Super::NativeDestruct();

	ACXGameStateBase* CXGameStateBase = GetWorld()->GetGameState<ACXGameStateBase>();
	if (CXGameStateBase == nullptr)
	{
		return;
	}
	if (CXGameStateBase->OnChangedCurrentGameState.IsAlreadyBound(this, &UGameStateInfoUI::OnChangedCurrentGameState) == false)
	{
		CXGameStateBase->OnChangedCurrentGameState.AddDynamic(this, &UGameStateInfoUI::OnChangedCurrentGameState);
	}
	if (CXGameStateBase->OnChangedRemainingTime.IsAlreadyBound(this, &UGameStateInfoUI::OnChangedRemainingTime) == true)
	{
		CXGameStateBase->OnChangedRemainingTime.RemoveDynamic(this, &UGameStateInfoUI::OnChangedRemainingTime);
	}
	if (CXGameStateBase->OnAnnouncedWinners.IsAlreadyBound(this, &UGameStateInfoUI::OnShowWinners) == true)
	{
		CXGameStateBase->OnAnnouncedWinners.RemoveDynamic(this, &UGameStateInfoUI::OnShowWinners);
	}
}

void UGameStateInfoUI::OnChangedCurrentGameState(EGameState NewGameState)
{
	if (IsValid(CurrentGameStateText) == false)
	{
		return;
	}

	FText DisplayName = UEnum::GetDisplayValueAsText(NewGameState);
	CurrentGameStateText->SetText(DisplayName);
}

void UGameStateInfoUI::OnChangedRemainingTime(int32 NewRemainingTime)
{
	if (IsValid(RemainingTimeText) == false)
	{
		return;
	}
	
	RemainingTimeText->SetText(FText::FromString(FString::Printf(TEXT("RemainingTime : %d"), NewRemainingTime)));
}

void UGameStateInfoUI::OnShowWinners(const TArray<ACXPlayerState*>& FinalWinners)
{
	if (IsValid(WinnerNamesText) == false)
	{
		return;
	}

	FString FinalString = FString(TEXT("Winner : "));
	for (int32 i = 0; i < FinalWinners.Num(); ++i)
	{
		FString PlayerName = FinalWinners[i]->GetPlayerName();
		FinalString.Append(PlayerName);

		if (i < FinalWinners.Num() - 1)
		{
			FinalString.Append(TEXT(", "));
		}
	}
	if (IsValid(RemainingTimeText))
	{
		RemainingTimeText->SetVisibility(ESlateVisibility::Collapsed);
	}
	WinnerNamesText->SetVisibility(ESlateVisibility::Visible);
	WinnerNamesText->SetText(FText::FromString(FinalString));
}
