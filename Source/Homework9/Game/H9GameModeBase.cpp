// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/H9GameModeBase.h"

#include "Game/h9GameStateBase.h"
#include "Controller/H9PlayerController.h"
#include "EngineUtils.h"
#include "Game/H9PlayerState.h"

AH9GameModeBase::AH9GameModeBase()
	: TurnDuration(30.0f)
	, CurrentTurnPlayerIndex(0)
	, GameStartDelay(5.0f)
	, bGameStarted(false)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AH9GameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	AH9PlayerController* H9PlayerController = Cast<AH9PlayerController>(NewPlayer);
	if (IsValid(H9PlayerController) == true)
	{
		H9PlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(H9PlayerController);
		
		AH9PlayerState* H9PS = H9PlayerController->GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			H9PS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		AH9GameStateBase* H9GameStateBase = GetGameState<AH9GameStateBase>();
		if (IsValid(H9GameStateBase) == true)
		{
			H9GameStateBase->MulticastRPCBroadcastLoginMessage(H9PS->PlayerNameString);
		}

		// 첫 번째 플레이어라면 게임 시작 타이머 설정
		if (AllPlayerControllers.Num() == 1 && bGameStarted == false)
		{
			GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AH9GameModeBase::StartGameAfterDelay, GameStartDelay, false);
		}
	}
}

FString AH9GameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) {return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool AH9GameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {
		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;

}

FString AH9GameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AH9GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Warning, TEXT("Secret Number: %s"), *SecretNumberString);
}

void AH9GameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTurnTime(DeltaSeconds);
}

void AH9GameModeBase::PrintChatMessageString(AH9PlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 플레이어가 추측할 수 있는지 확인
	if (CanPlayerGuess(InChattingPlayerController) == false)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("It's not your turn or you have no time left!"));
		return;
	}

	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);

		IncreaseGuessCount(InChattingPlayerController);

		// 이번 턴에 플레이했다고 표시
		AH9PlayerState* H9PS = InChattingPlayerController->GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			H9PS->bHasPlayedThisTurn = true;
		}

		for (TActorIterator<AH9PlayerController> It(GetWorld()); It; ++It)
		{
			AH9PlayerController* H9PlayerController = *It;
			if (IsValid(H9PlayerController) == true)
			{
				FString CombinedChatMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				H9PlayerController->ClientRPCPrintChatMessageString(CombinedChatMessageString);
			}
		}

		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));

		bool bGameEnded = JudgeGame(InChattingPlayerController, StrikeCount);

		// 게임이 끝나지 않았을 때만 다음 턴으로 넘김
		if (bGameEnded == false)
		{
			StartNextTurn();
		}
	}
	else
	{
		for (TActorIterator<AH9PlayerController> It(GetWorld()); It; ++It)
		{
			AH9PlayerController* H9PlayerController = *It;
			if (IsValid(H9PlayerController) == true)
			{
				H9PlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void AH9GameModeBase::IncreaseGuessCount(AH9PlayerController* InChattingPlayerController)
{
	AH9PlayerState* H9PS = InChattingPlayerController->GetPlayerState<AH9PlayerState>();
	if(IsValid(H9PS) == true)
	{
		H9PS->CurrentGuessCount++;
	}
}

void AH9GameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (const auto& H9PlayerController : AllPlayerControllers)
	{
		AH9PlayerState* H9PS = H9PlayerController->GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			H9PS->CurrentGuessCount = 0;
			H9PS->bHasPlayedThisTurn = false;
		}
	}

	CurrentTurnPlayerIndex = 0;
	
	// 게임 재시작 시에도 대기 시간 적용
	bGameStarted = false;
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, 
		&AH9GameModeBase::StartGameAfterDelay, GameStartDelay, false);
}

bool AH9GameModeBase::JudgeGame(AH9PlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		AH9PlayerState* H9PS = InChattingPlayerController->GetPlayerState<AH9PlayerState>();
		for (const auto& H9PlayerController : AllPlayerControllers)
		{
			if (IsValid(H9PS) == true)
			{
				FString CombinedMessageString = H9PS->PlayerNameString + TEXT(" has won the game!");
				H9PlayerController->NotificationText = FText::FromString(CombinedMessageString);
			}
		}
		ResetGame();
		return true;

	}
	else
	{
		bool bIsDraw = true;
		for (const auto& H9PlayerController : AllPlayerControllers)
		{
			AH9PlayerState* H9PS = H9PlayerController->GetPlayerState<AH9PlayerState>();
			if (IsValid(H9PS) == true)
			{
				if (H9PS->CurrentGuessCount < H9PS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& H9PlayerController : AllPlayerControllers)
			{
				H9PlayerController->NotificationText = FText::FromString(TEXT("The game is a draw!"));
			}
			ResetGame();
			return true;
		}
	}

	// 게임이 끝나지 않음
	return false;
}

void AH9GameModeBase::StartNextTurn()
{
	if (AllPlayerControllers.Num() == 0)
	{
		return;
	}

	// 모든 플레이어의 턴 상태 초기화
	for (const auto& H9PlayerController : AllPlayerControllers)
	{
		AH9PlayerState* H9PS = H9PlayerController->GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			H9PS->bIsMyTurn = false;
			H9PS->RemainingTurnTime = 0.0f;
		}
	}

	// 다음 플레이어 찾기
	int32 StartIndex = (CurrentTurnPlayerIndex + 1) % AllPlayerControllers.Num();
	bool bFoundPlayer = false;

	for (int32 i = 0; i < AllPlayerControllers.Num(); ++i)
	{
		int32 CheckIndex = (StartIndex + i) % AllPlayerControllers.Num();
		AH9PlayerState* H9PS = AllPlayerControllers[CheckIndex]->GetPlayerState<AH9PlayerState>();
		
		if (IsValid(H9PS) == true && H9PS->CurrentGuessCount < H9PS->MaxGuessCount)
		{
			CurrentTurnPlayerIndex = CheckIndex;
			H9PS->bIsMyTurn = true;
			H9PS->RemainingTurnTime = TurnDuration;
			H9PS->bHasPlayedThisTurn = false;
			bFoundPlayer = true;
			
			// 모든 플레이어에게 턴 변경 알림
			for (int32 j = 0; j < AllPlayerControllers.Num(); ++j)
			{
				if (j == CheckIndex)
				{
					// 현재 턴 플레이어에게 알림
					AllPlayerControllers[j]->NotificationText = FText::FromString(TEXT("Your turn!"));
				}
				else
				{
					// 다른 플레이어들에게 대기 알림
					FString WaitingMessage = H9PS->PlayerNameString + TEXT("'s turn. Waiting...");
					AllPlayerControllers[j]->NotificationText = FText::FromString(WaitingMessage);
				}
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Turn switched to %s (Index: %d)"), *H9PS->PlayerNameString, CheckIndex);
			break;
		}
	}

	// 모든 플레이어가 기회를 다 썼다면 게임 종료
	if (bFoundPlayer == false)
	{
		for (const auto& H9PlayerController : AllPlayerControllers)
		{
			H9PlayerController->NotificationText = FText::FromString(TEXT("The game is a draw!"));
		}
		ResetGame();
	}
}

void AH9GameModeBase::UpdateTurnTime(float DeltaSeconds)
{
	if (AllPlayerControllers.Num() == 0 || CurrentTurnPlayerIndex >= AllPlayerControllers.Num())
	{
		return;
	}

	AH9PlayerState* CurrentPlayerState = AllPlayerControllers[CurrentTurnPlayerIndex]->GetPlayerState<AH9PlayerState>();
	if (IsValid(CurrentPlayerState) == true && CurrentPlayerState->bIsMyTurn == true)
	{
		CurrentPlayerState->RemainingTurnTime -= DeltaSeconds;

		if (CurrentPlayerState->RemainingTurnTime <= 0.0f)
		{
			OnTurnTimeout();
		}
	}
}

void AH9GameModeBase::OnTurnTimeout()
{
	if (AllPlayerControllers.Num() == 0 || CurrentTurnPlayerIndex >= AllPlayerControllers.Num())
	{
		return;
	}

	AH9PlayerController* CurrentPlayer = AllPlayerControllers[CurrentTurnPlayerIndex];
	AH9PlayerState* H9PS = CurrentPlayer->GetPlayerState<AH9PlayerState>();

	if (IsValid(H9PS) == true)
	{
		// 턴 내에 플레이하지 않았다면 기회 소진
		if (H9PS->bHasPlayedThisTurn == false)
		{
			H9PS->CurrentGuessCount++;
			
			for (const auto& H9PlayerController : AllPlayerControllers)
			{
				FString TimeoutMessage = H9PS->PlayerNameString + TEXT(" timed out! Chance lost.");
				H9PlayerController->ClientRPCPrintChatMessageString(TimeoutMessage);
			}
		}

		H9PS->RemainingTurnTime = 0.0f;
	}

	// 다음 턴으로
	StartNextTurn();
}

bool AH9GameModeBase::CanPlayerGuess(AH9PlayerController* InPlayerController)
{
	if (IsValid(InPlayerController) == false)
	{
		return false;
	}

	AH9PlayerState* H9PS = InPlayerController->GetPlayerState<AH9PlayerState>();
	if (IsValid(H9PS) == false)
	{
		return false;
	}

	// 게임이 시작되지 않았으면 플레이 불가
	if (bGameStarted == false)
	{
		return false;
	}

	// 자기 턴이고, 시간이 남아있고, 기회가 남아있어야 함
	return H9PS->bIsMyTurn && H9PS->RemainingTurnTime > 0.0f && H9PS->CurrentGuessCount < H9PS->MaxGuessCount;
}

void AH9GameModeBase::StartGameAfterDelay()
{
	bGameStarted = true;
	StartNextTurn();
	
	UE_LOG(LogTemp, Warning, TEXT("Game started!"));
}

