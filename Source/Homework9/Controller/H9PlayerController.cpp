// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/H9PlayerController.h"

#include "UI/H9ChatInput.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Game/H9GameModeBase.h"
#include "Game/H9PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

AH9PlayerController::AH9PlayerController()
{
	bReplicates = true;
}

void AH9PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UH9ChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(TurnTimerWidgetClass) == true)
	{
		TurnTimerWidgetInstance = CreateWidget<UUserWidget>(this, TurnTimerWidgetClass);
		if (IsValid(TurnTimerWidgetInstance) == true)
		{
			TurnTimerWidgetInstance->AddToViewport();
		}
	}

}

void AH9PlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		AH9PlayerState* H9PS = GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			FString CombinedMessageString = H9PS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void AH9PlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	UKismetSystemLibrary::PrintString(this, InChatMessageString, true, true, FLinearColor::Red, 5.0f);
}

void AH9PlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, NotificationText);
}

void AH9PlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void AH9PlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		AH9GameModeBase* H9GM = Cast<AH9GameModeBase>(GM);
		if (IsValid(H9GM) == true)
		{
			H9GM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}
