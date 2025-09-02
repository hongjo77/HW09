// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NBGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/NBPlayerController.h"
#include "Net/UnrealNetwork.h"

void ANBGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ANBPlayerController* NBPC = Cast<ANBPlayerController>(PC);
			if (IsValid(NBPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				NBPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
