// NBTurnTimerWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBTurnTimerWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class NUMBASEBALL_API UNBTurnTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Timer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Timer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_CurrentPlayer;

	void UpdateTimerDisplay();
};
