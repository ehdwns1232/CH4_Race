// Fill out your copyright notice in the Description page of Project Settings.


#include "CXPlayerState.h"
#include "Net/UnrealNetwork.h"

static int32 GMyTestCoin = 0;
static FAutoConsoleVariableRef CVarAddCoinServer(TEXT("Cheat.addtion.server"), GMyTestCoin, TEXT("Add Coin request to server"),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* CVar)
		{
			UE_LOG(LogTemp, Warning, TEXT("콘솔 변수 변경됨 ! GMyTestCoin 값: %d"), GMyTestCoin);

			if (GWorld)
			{
				if (GWorld->GetFirstPlayerController())
				{
					GWorld->GetFirstPlayerController()->GetPlayerState<ACXPlayerState>()->CTSAddCoin(GMyTestCoin);
				}
			}
			GMyTestCoin = 0;
		}),
	ECVF_Cheat
);

static FAutoConsoleVariableRef CVarAddCoinClinet(TEXT("Cheat.addtion.client"), GMyTestCoin, TEXT("Add Coin request to client only"),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* CVar)
		{
			UE_LOG(LogTemp, Warning, TEXT("콘솔 변수 변경됨 ! GMyTestCoin 값: %d"), GMyTestCoin);

			if (GWorld)
			{
				if (GWorld->GetFirstPlayerController())
				{
					GWorld->GetFirstPlayerController()->GetPlayerState<ACXPlayerState>()->Coins += GMyTestCoin;
				}
			}
			GMyTestCoin = 0;
		}),
	ECVF_Cheat
);


void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, BonusMaxSpeed, COND_OwnerOnly);
}

void ACXPlayerState::CTSAddCoin_Implementation(const int32 Amount)
{

	AddCoins(Amount);
}

void ACXPlayerState::AddCoins(int32 Amount)
{
	if (HasAuthority() == false || Amount <= 0)
	{
		return;
	}
	FString PlayerName = GetPlayerName();
	UE_LOG(LogTemp, Warning, TEXT("%s - AddCoins: %d -> %d"), *PlayerName, Coins, Coins + Amount);
	Coins += Amount;
}

bool ACXPlayerState::TryRemoveCoins(int32 Amount)
{
	if (HasAuthority() == false || Amount <= 0 || Coins < 5)
	{
		return false;
	}

	Coins -= Amount;
	return true;
}

void ACXPlayerState::AddBonusMaxSpeed(float Amount)
{
	if (HasAuthority() == false || Amount <= 0.f)
	{
		return;
	}

	BonusMaxSpeed += Amount;
}

void ACXPlayerState::ApplyPenalty()
{
	if (HasAuthority() == false)
	{
		return;
	}

	BonusMaxSpeed = PanaltyAmount;

	GetWorldTimerManager().SetTimer(PenaltyHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			BonusMaxSpeed = FMath::Clamp(BonusMaxSpeed + 10.0f, PanaltyAmount, 0.0f);
			if (FMath::IsNearlyZero(BonusMaxSpeed))
			{
				BonusMaxSpeed = 0.0f;
				GetWorldTimerManager().ClearTimer(PenaltyHandle);
			}
		}), 1.0f, true);
}

FString ACXPlayerState::GetPlayerStateInfo() const
{
	return FString::Printf(TEXT("Coins: %d, BonusMaxSpeed: %.2f"), Coins, BonusMaxSpeed);
}
