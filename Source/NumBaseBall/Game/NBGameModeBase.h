// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBGameModeBase.generated.h"

class ANBPlayerController;
/**
 * 
 */
UCLASS

()
class NUMBASEBALL_API ANBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANBGameModeBase();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	void PrintChatMessageString(ANBPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	void IncreaseGuessCount(ANBPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ANBPlayerController* InChattingPlayerController, int InStrikeCount);

	void StartNextTurn();
	void EndCurrentTurn();
	void HandleTurnTimeout();
	void SetCurrentPlayer(ANBPlayerController* NewCurrentPlayer);

protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ANBPlayerController>> AllPlayerControllers;

	UPROPERTY()
	TObjectPtr<ANBPlayerController> CurrentTurnPlayer;

	UPROPERTY()
	int32 CurrentPlayerIndex;

	UPROPERTY()
	float TurnTimeLimit;

	UPROPERTY()
	float CurrentTurnTime;

	UPROPERTY()
	bool bGameActive;

	FTimerHandle TurnTimerHandle;
};
