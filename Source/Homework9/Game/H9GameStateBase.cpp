// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/H9GameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Controller/H9PlayerController.h"

void AH9GameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			AH9PlayerController* H9PC = Cast<AH9PlayerController>(PC);
			if (IsValid(H9PC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has logged in.");
				H9PC->PrintChatMessageString(NotificationString);

			}
		}
	}
}
