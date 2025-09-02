// NBTurnTimerWidget.cpp
#include "UI/NBTurnTimerWidget.h"

#include "EngineUtils.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Player/NBPlayerController.h"
#include "Player/NBPlayerState.h"

void UNBTurnTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateTimerDisplay();
}

void UNBTurnTimerWidget::UpdateTimerDisplay()
{
    APlayerController* PC = GetOwningPlayer();
    if (IsValid(PC))
    {
        UWorld* World = PC->GetWorld();
        if (IsValid(World))
        {
            // 현재 턴인 플레이어 찾기
            ANBPlayerController* CurrentTurnPlayer = nullptr;
            float RemainingTime = 0.0f;
            
            for (TActorIterator<ANBPlayerController> It(World); It; ++It)
            {
                ANBPlayerController* NBPlayerController = *It;
                if (IsValid(NBPlayerController))
                {
                    ANBPlayerState* NBPS = NBPlayerController->GetPlayerState<ANBPlayerState>();
                    if (IsValid(NBPS) && NBPS->bIsMyTurn)
                    {
                        CurrentTurnPlayer = NBPlayerController;
                        RemainingTime = NBPS->RemainingTurnTime;
                        break;
                    }
                }
            }

            if (IsValid(CurrentTurnPlayer))
            {
                ANBPlayerState* NBPS = CurrentTurnPlayer->GetPlayerState<ANBPlayerState>();
                if (IsValid(NBPS))
                {
                    // 타이머 텍스트 업데이트
                    int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
                    int32 Seconds = FMath::FloorToInt(RemainingTime) % 60;
                    FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
                    
                    if (IsValid(TextBlock_Timer))
                    {
                        TextBlock_Timer->SetText(FText::FromString(TimeString));
                    }

                    // 프로그레스 바 업데이트
                    float Progress = RemainingTime / 20.0f;
                    if (IsValid(ProgressBar_Timer))
                    {
                        ProgressBar_Timer->SetPercent(Progress);
                    }

                    // 현재 플레이어 텍스트 업데이트
                    ANBPlayerController* NBPC = Cast<ANBPlayerController>(PC);
                    if (IsValid(NBPC))
                    {
                        ANBPlayerState* MyPlayerState = NBPC->GetPlayerState<ANBPlayerState>();
                        if (IsValid(MyPlayerState))
                        {
                            FString CurrentPlayerText;
                            if (MyPlayerState->bIsMyTurn)
                            {
                                CurrentPlayerText = TEXT("Your Turn!");
                            }
                            else
                            {
                                CurrentPlayerText = NBPS->PlayerNameString + TEXT("'s Turn - Wait your turn");
                            }
                            
                            if (IsValid(TextBlock_CurrentPlayer))
                            {
                                TextBlock_CurrentPlayer->SetText(FText::FromString(CurrentPlayerText));
                            }
                        }
                    }
                }
            }
            else
            {
                if (IsValid(TextBlock_Timer))
                {
                    TextBlock_Timer->SetText(FText::FromString(TEXT("00:00")));
                }
                if (IsValid(ProgressBar_Timer))
                {
                    ProgressBar_Timer->SetPercent(0.0f);
                }
                if (IsValid(TextBlock_CurrentPlayer))
                {
                    TextBlock_CurrentPlayer->SetText(FText::FromString(TEXT("Waiting for players...")));
                }
            }
        }
    }
}
