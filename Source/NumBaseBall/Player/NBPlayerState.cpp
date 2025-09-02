// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NBPlayerState.h"
#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState()
	: PlayerNameString(TEXT("None"))
	, CurrentGuessCount(0)
	, MaxGuessCount(3)
	, bIsMyTurn(false)
	, bHasPlayedThisTurn(false)
	, RemainingTurnTime(0.0f)
{
	bReplicates = true;
}

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerNameString);
	DOREPLIFETIME(ThisClass, CurrentGuessCount);
	DOREPLIFETIME(ThisClass, MaxGuessCount);
	DOREPLIFETIME(ThisClass, bIsMyTurn);
	DOREPLIFETIME(ThisClass, bHasPlayedThisTurn);
	DOREPLIFETIME(ThisClass, RemainingTurnTime);
}

FString ANBPlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = PlayerNameString + TEXT("(") + FString::FromInt(CurrentGuessCount) + TEXT("/") + FString::FromInt(MaxGuessCount) + TEXT(")");
	return PlayerInfoString;
}
