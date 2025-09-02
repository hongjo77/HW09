#include "Game/NBGameModeBase.h"
#include "NBGameStateBase.h"
#include "Player/NBPlayerController.h"
#include "EngineUtils.h"
#include "Player/NBPlayerState.h"

ANBGameModeBase::ANBGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ANBGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ANBPlayerController* NBPlayerController = Cast<ANBPlayerController>(NewPlayer);
	if (IsValid(NBPlayerController) == true)
	{
		NBPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		AllPlayerControllers.Add(NBPlayerController);

		ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
		if (IsValid(NBPS) == true)
		{
			NBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
			NBPS->bIsMyTurn = false;
			NBPS->bHasPlayedThisTurn = false;
			NBPS->RemainingTurnTime = 0.0f;
		}

		ANBGameStateBase* NBGameStateBase =  GetGameState<ANBGameStateBase>();
		if (IsValid(NBGameStateBase) == true)
		{
			NBGameStateBase->MulticastRPCBroadcastLoginMessage(NBPS->PlayerNameString);
		}
		// 게임이 활성화되지 않았고 플레이어가 2명 이상이면 게임 시작
		if (!bGameActive && AllPlayerControllers.Num() >= 2)
		{
			bGameActive = true;
			StartNextTurn();
		}
	}
}

FString ANBGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });
	
	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ANBGameModeBase::IsGuessNumberString(const FString& InNumberString)
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

FString ANBGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
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

void ANBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	CurrentPlayerIndex = 0;
	TurnTimeLimit = 20.0f;
	CurrentTurnTime = TurnTimeLimit;
	bGameActive = false;
}

void ANBGameModeBase::PrintChatMessageString(ANBPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 현재 턴이 아닌 플레이어는 게임 플레이 불가
	if (CurrentTurnPlayer != InChattingPlayerController)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("It's not your turn!"));
		return;
	}
	ANBPlayerState* NBPS = InChattingPlayerController->GetPlayerState<ANBPlayerState>();
	if (IsValid(NBPS) && NBPS->CurrentGuessCount >= NBPS->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("It's your turn!"));
		return;
	}
	
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true)
	{
		// 이번 턴에 플레이했다고 표시
		if (IsValid(NBPS))
		{
			NBPS->bHasPlayedThisTurn = true;
		}
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		IncreaseGuessCount(InChattingPlayerController);
		for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
		{
			ANBPlayerController* NBPlayerController = *It;
			if (IsValid(NBPlayerController) == true)
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				NBPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}
		int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		JudgeGame(InChattingPlayerController, StrikeCount);
		// 게임이 끝나지 않았다면 다음 턴으로
		if (StrikeCount != 3)
		{
			EndCurrentTurn();
		}
	}
	else
	{
		for (TActorIterator<ANBPlayerController> It(GetWorld()); It; ++It)
		{
			ANBPlayerController* NBPlayerController = *It;
			if (IsValid(NBPlayerController) == true)
			{
				NBPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void ANBGameModeBase::IncreaseGuessCount(ANBPlayerController* InChattingPlayerController)
{
	ANBPlayerState* NBPS = InChattingPlayerController->GetPlayerState<ANBPlayerState>();
	if (IsValid(NBPS) == true)
	{
		NBPS->CurrentGuessCount++;
	}
}

void ANBGameModeBase::StartNextTurn()
{
    if (AllPlayerControllers.Num() == 0) return;

    ANBPlayerController* NextPlayer = nullptr;
    int32 StartIndex = CurrentPlayerIndex;
    
    for (int32 i = 0; i < AllPlayerControllers.Num(); ++i)
    {
        int32 TestIndex = (StartIndex + 1 + i) % AllPlayerControllers.Num();
        ANBPlayerController* TestPlayer = AllPlayerControllers[TestIndex];
        
        if (IsValid(TestPlayer))
        {
            ANBPlayerState* TestNBPS = TestPlayer->GetPlayerState<ANBPlayerState>();
            if (IsValid(TestNBPS) && TestNBPS->CurrentGuessCount < TestNBPS->MaxGuessCount)
            {
                CurrentPlayerIndex = TestIndex;
                NextPlayer = TestPlayer;
                break;
            }
        }
    }

    if (!IsValid(NextPlayer))
    {
        bGameActive = false;
        for (const auto& NBPlayerController : AllPlayerControllers)
        {
            NBPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
            
            ANBPlayerState* PlayerNBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
            if (IsValid(PlayerNBPS))
            {
                PlayerNBPS->bIsMyTurn = false;
            }
        }
        
        GetWorld()->GetTimerManager().SetTimer(TurnTimerHandle, this, &ANBGameModeBase::ResetGame, 5.0f, false);
        return;
    }

    CurrentTurnPlayer = NextPlayer;
    CurrentTurnTime = TurnTimeLimit;

    // 모든 플레이어에게 턴 변경 알림
    for (const auto& NBPlayerController : AllPlayerControllers)
    {
        if (IsValid(NBPlayerController))
        {
            ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
            if (IsValid(NBPS))
            {
                NBPS->bIsMyTurn = (NBPlayerController == CurrentTurnPlayer);
                NBPS->bHasPlayedThisTurn = false;
                
                if (NBPS->bIsMyTurn)
                {
                    FString YourTurnMessage = FString::Printf(TEXT("Your turn! You have %.0f seconds."), TurnTimeLimit);
                    NBPlayerController->NotificationText = FText::FromString(YourTurnMessage);
                }
                else
                {
                    FString TurnMessage = CurrentTurnPlayer->GetPlayerState<ANBPlayerState>()->PlayerNameString + TEXT("'s turn - Wait your turn");
                    NBPlayerController->NotificationText = FText::FromString(TurnMessage);
                }
            }
        }
    }
}

void ANBGameModeBase::EndCurrentTurn()
{
    StartNextTurn();
}

void ANBGameModeBase::HandleTurnTimeout()
{
    if (IsValid(CurrentTurnPlayer))
    {
        ANBPlayerState* NBPS = CurrentTurnPlayer->GetPlayerState<ANBPlayerState>();
        if (IsValid(NBPS))
        {
            // 이번 턴에 플레이하지 않았다면 기회 소진
            if (!NBPS->bHasPlayedThisTurn)
            {
                NBPS->CurrentGuessCount++;
                
                // 모든 플레이어에게 타임아웃 알림
                for (const auto& NBPlayerController : AllPlayerControllers)
                {
                    if (IsValid(NBPlayerController))
                    {
                        FString TimeoutMessage = NBPS->PlayerNameString + TEXT(" timed out! Chance lost.");
                        NBPlayerController->ClientRPCPrintChatMessageString(TimeoutMessage);
                    }
                }
            }
        }
    }

    EndCurrentTurn();
}

void ANBGameModeBase::SetCurrentPlayer(ANBPlayerController* NewCurrentPlayer)
{
    CurrentTurnPlayer = NewCurrentPlayer;
}

void ANBGameModeBase::ResetGame()
{
    SecretNumberString = GenerateSecretNumber();
    CurrentPlayerIndex = -1;
    bGameActive = true;

    for (const auto& NBPlayerController : AllPlayerControllers)
    {
        ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
        if (IsValid(NBPS))
        {
            NBPS->CurrentGuessCount = 0;
            NBPS->bIsMyTurn = false;
            NBPS->bHasPlayedThisTurn = false;
        }
    }

    StartNextTurn();
}

void ANBGameModeBase::JudgeGame(ANBPlayerController* InChattingPlayerController, int InStrikeCount)
{
    if (3 == InStrikeCount)
    {
        bGameActive = false;
        ANBPlayerState* NBPS = InChattingPlayerController->GetPlayerState<ANBPlayerState>();
        for (const auto& NBPlayerController : AllPlayerControllers)
        {
            if (IsValid(NBPS))
            {
                FString CombinedMessageString = NBPS->PlayerNameString + TEXT(" has won the game.");
                NBPlayerController->NotificationText = FText::FromString(CombinedMessageString);
                
                // 모든 플레이어의 턴 종료
                ANBPlayerState* PlayerNBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
                if (IsValid(PlayerNBPS))
                {
                    PlayerNBPS->bIsMyTurn = false;
                }
            }
        }
        
        // 5초 후 게임 재시작
        GetWorld()->GetTimerManager().SetTimer(TurnTimerHandle, this, &ANBGameModeBase::ResetGame, 5.0f, false);
    }
    else
    {
        bool bIsDraw = true;
        for (const auto& NBPlayerController : AllPlayerControllers)
        {
            ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
            if (IsValid(NBPS))
            {
                if (NBPS->CurrentGuessCount < NBPS->MaxGuessCount)
                {
                    bIsDraw = false;
                    break;
                }
            }
        }

        if (bIsDraw)
        {
            bGameActive = false;
            for (const auto& NBPlayerController : AllPlayerControllers)
            {
                NBPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
                
                ANBPlayerState* PlayerNBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
                if (IsValid(PlayerNBPS))
                {
                    PlayerNBPS->bIsMyTurn = false;
                }
            }
            
            // 5초 후 게임 재시작
            GetWorld()->GetTimerManager().SetTimer(TurnTimerHandle, this, &ANBGameModeBase::ResetGame, 5.0f, false);
        }
    }
}

void ANBGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bGameActive && IsValid(CurrentTurnPlayer))
	{
		CurrentTurnTime -= DeltaTime;
        
		// 현재 플레이어의 남은 시간 업데이트
		ANBPlayerState* NBPS = CurrentTurnPlayer->GetPlayerState<ANBPlayerState>();
		if (IsValid(NBPS))
		{
			NBPS->RemainingTurnTime = FMath::Max(0.0f, CurrentTurnTime);
          
		}

		if (CurrentTurnTime <= 0.0f)
		{
			HandleTurnTimeout();
		}
	}
}