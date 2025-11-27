// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/H9PlayerState.h"

#include "Net/UnrealNetwork.h"

AH9PlayerState::AH9PlayerState()
	: PlayerNameString(TEXT("None"))
	, CurrentGuessCount(0)
	, MaxGuessCount(3)
	, RemainingTurnTime(0.0f)
	, bHasPlayedThisTurn(false)
	, bIsMyTurn(false)
{
	bReplicates = true;
}

void AH9PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
	DOREPLIFETIME(ThisClass, RemainingTurnTime);
	DOREPLIFETIME(ThisClass, bHasPlayedThisTurn);
	DOREPLIFETIME(ThisClass, bIsMyTurn);
}

FString AH9PlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = PlayerNameString + TEXT(" (") + FString::FromInt(CurrentGuessCount+1) + TEXT("/") + FString::FromInt(MaxGuessCount) + TEXT(")");
	return PlayerInfoString;
}