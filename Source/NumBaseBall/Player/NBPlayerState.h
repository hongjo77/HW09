// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NUMBASEBALL_API ANBPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANBPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UPROPERTY(Replicated)
	FString PlayerNameString;

	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsMyTurn;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bHasPlayedThisTurn;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float RemainingTurnTime;

	FString GetPlayerInfoString();
};
