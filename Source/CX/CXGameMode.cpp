// CXGameMode.cpp

#include "CXGameMode.h"
#include "CXPlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "CXSubsystem/ChatSubsystem.h"
#include "CXPlayerState.h"
#include "CXGameStateBase.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	PlayerStateClass = ACXPlayerState::StaticClass();
	GameStateClass = ACXGameStateBase::StaticClass();
}

void ACXGameMode::BeginPlay()
{
	Super::BeginPlay();

	CXGameStateCached = GetGameState<ACXGameStateBase>();
	if (CXGameStateCached.IsValid() == true)
	{
		CXGameStateCached->SetGameState(EGameState::Waiting);
		CXGameStateCached->SetRemainingTime(WaitingDuration);
	}

	bIsWaitingTimeStarted = false;
}

void ACXGameMode::StartWaitingCountdown()
{
	if (CXGameStateCached.IsValid() == true)
	{
		CXGameStateCached->SetGameState(EGameState::Waiting);
		CXGameStateCached->SetRemainingTime(WaitingDuration);
		GetWorldTimerManager().SetTimer(WaitingTimerHandle, this, &ACXGameMode::HandleWatingTimerTick, 1.0f, true);
		bIsWaitingTimeStarted = true;
	}
}

void ACXGameMode::HandleWatingTimerTick()
{
	const int32 NewRemainingTime = FMath::Clamp(CXGameStateCached->GetRemainingTime() - 1, 0, WaitingDuration);
	CXGameStateCached->SetRemainingTime(NewRemainingTime);

	if (NewRemainingTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(WaitingTimerHandle);
		StartPlayingState();
	}
}

void ACXGameMode::StartPlayingState()
{
	if (CXGameStateCached.IsValid() == true)
	{
		CXGameStateCached->SetGameState(EGameState::Playing);
		CXGameStateCached->SetRemainingTime(PlayingDuration);
		GetWorldTimerManager().SetTimer(PlayingTimerHandle, this, &ACXGameMode::HandlePlayingTimerTick, 1.0f, true);
	}
}

void ACXGameMode::HandlePlayingTimerTick()
{
	const int32 DeltaRemainingTime = FMath::Clamp(CXGameStateCached->GetRemainingTime() - 1, 0, PlayingDuration);
	CXGameStateCached->SetRemainingTime(DeltaRemainingTime);

	if (DeltaRemainingTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(PlayingTimerHandle);
		FinishGame();
	}
}

void ACXGameMode::FinishGame()
{
	if (CXGameStateCached.IsValid() == true)
	{
		int32 HighestCoins = -1;
		TArray<ACXPlayerState*> Winners;

		TArray<APlayerState*> PlayerStateArray = CXGameStateCached->PlayerArray;
		for (APlayerState* PlayerState : PlayerStateArray)
		{
			if (ACXPlayerState* CXPlayerState = Cast<ACXPlayerState>(PlayerState))
			{
				const int32 PlayerCoins = CXPlayerState->GetCoins();
				if (HighestCoins < PlayerCoins)
				{
					HighestCoins = PlayerCoins;
					Winners.Empty();
					Winners.Add(CXPlayerState);
				}
				else if (HighestCoins == PlayerCoins)
				{
					Winners.Add(CXPlayerState);
				}
			}
		}
		CXGameStateCached->SetWinners(Winners);
		CXGameStateCached->SetRemainingTime(0);
		CXGameStateCached->SetGameState(EGameState::Finished);
	}
}

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (IsValid(NewPlayer) == false)
	{
		return;
	}

	if (ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer))
	{
		if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
		{
			const TArray<FChatData> ChatLog = ChatSubsystem->GetChatLog();
			if (!ChatLog.IsEmpty())
			{
				CXPlayerController->STCChatLog(ChatLog);
			}
		}
	}

	if (bIsWaitingTimeStarted == false)
	{
		StartWaitingCountdown();
	}
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(VehicleSpawnCheckOffset);
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(VehicleSpawnCheckHalfExtent);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}