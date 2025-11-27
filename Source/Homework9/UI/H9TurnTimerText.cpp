// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/H9TurnTimerText.h"
#include "Game/H9PlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void UH9TurnTimerText::NativeConstruct()
{
	Super::NativeConstruct();

	MaxTurnDuration = 30.0f;
}

void UH9TurnTimerText::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 매 프레임마다 타이머 업데이
}

FText UH9TurnTimerText::GetRemainingTimeText() const
{
	AH9PlayerState* H9PS = GetLocalPlayerState();
	if (IsValid(H9PS) == false || H9PS->bIsMyTurn == false)
	{
		return FText::FromString(TEXT("--"));
	}

	float RemainingTime = FMath::Max(0.0f, H9PS->RemainingTurnTime);
	int32 Seconds = FMath::CeilToInt(RemainingTime);
	
	return FText::FromString(FString::Printf(TEXT("%d"), Seconds));
}

bool UH9TurnTimerText::IsMyTurn() const
{
	AH9PlayerState* H9PS = GetLocalPlayerState();
	if (IsValid(H9PS) == false)
	{
		return false;
	}

	return H9PS->bIsMyTurn;
}

float UH9TurnTimerText::GetRemainingTimePercent() const
{
	AH9PlayerState* H9PS = GetLocalPlayerState();
	if (IsValid(H9PS) == false || MaxTurnDuration <= 0.0f || H9PS->bIsMyTurn == false)
	{
		return 0.0f;
	}

	float RemainingTime = FMath::Max(0.0f, H9PS->RemainingTurnTime);
	return RemainingTime / MaxTurnDuration;
}

AH9PlayerState* UH9TurnTimerText::GetLocalPlayerState() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (IsValid(PC) == false)
	{
		return nullptr;
	}

	return PC->GetPlayerState<AH9PlayerState>();
}

