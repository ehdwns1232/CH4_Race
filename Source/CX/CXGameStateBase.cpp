// Fill out your copyright notice in the Description page of Project Settings.


#include "CXGameStateBase.h"
#include "Net/UnrealNetwork.h"

void ACXGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentGameState);
	DOREPLIFETIME(ThisClass, RemainingTime);
	DOREPLIFETIME(ThisClass, Winners);
}

void ACXGameStateBase::SetGameState(EGameState NewGameState)
{
	if (HasAuthority() == false || CurrentGameState == NewGameState)
	{
		return;
	}

	CurrentGameState = NewGameState;
}

void ACXGameStateBase::SetRemainingTime(int32 NewRemainingTime)
{
	if (HasAuthority() == false)
	{
		return;
	}

	RemainingTime = FMath::Max(0, NewRemainingTime);
}

void ACXGameStateBase::SetWinners(const TArray<ACXPlayerState*>& FinalWinners)
{
	if (HasAuthority() == false)
	{
		return;
	}

	Winners = FinalWinners;
}

void ACXGameStateBase::OnRep_ChangedCurrentGameState()
{
	OnChangedCurrentGameState.Broadcast(CurrentGameState);
}

void ACXGameStateBase::OnRep_ChangedRemainingTime()
{
	OnChangedRemainingTime.Broadcast(RemainingTime);
}

void ACXGameStateBase::OnRep_AnnouncedWinners()
{
	OnAnnouncedWinners.Broadcast(Winners);
}
