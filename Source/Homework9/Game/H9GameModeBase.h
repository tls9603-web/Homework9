// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "H9GameModeBase.generated.h"

class AH9PlayerController;

/**
 * 
 */
UCLASS()
class HOMEWORK9_API AH9GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AH9GameModeBase();

	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void PrintChatMessageString(AH9PlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void IncreaseGuessCount(AH9PlayerController* InChattingPlayerController);

	void ResetGame();

	bool JudgeGame(AH9PlayerController* InChattingPlayerController, int InStrikeCount);

	// 턴 관리 함수
	void StartNextTurn();
	void UpdateTurnTime(float DeltaSeconds);
	void OnTurnTimeout();
	bool CanPlayerGuess(AH9PlayerController* InPlayerController);

	// 게임 시작 함수
	void StartGameAfterDelay();

protected:
	FString SecretNumberString;

	TArray<TObjectPtr<AH9PlayerController>> AllPlayerControllers;

	// 턴 관리 변수
	UPROPERTY(EditDefaultsOnly, Category = "Turn")
	float TurnDuration;

	int32 CurrentTurnPlayerIndex;

	// 게임 시작 대기 변수
	UPROPERTY(EditDefaultsOnly, Category = "Game")
	float GameStartDelay;  // 5.0초

	bool bGameStarted;  // 게임 시작 여부

	FTimerHandle GameStartTimerHandle;  // 타이머 핸들
};
