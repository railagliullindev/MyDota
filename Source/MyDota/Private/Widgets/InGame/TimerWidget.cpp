// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/TimerWidget.h"

#include "Components/TextBlock.h"
#include "FuncLibraries/MD_UIHelper.h"
#include "GameFrameworks/MD_GameState.h"

void UTimerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGS = GetWorld()->GetGameState<AMD_GameState>();

	if (!CachedGS)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTimerWidget Cant find GS"));
		return;
	}
	OnGameTimeChanged();
	CachedGS->OnGameTimeChanged.AddDynamic(this, &UTimerWidget::OnGameTimeChanged);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UTimerWidget::TimeTick, 1.f, true, 0.f);
}

void UTimerWidget::OnGameTimeChanged()
{
	CurrentGameTime = CachedGS->GetServerWorldTimeSeconds();
	GameTargetTime = CachedGS->MatchStartTime;
	UE_LOG(LogTemp, Warning, TEXT("UTimerWidget::OnGameTimeChanged() %f "), GameTargetTime);
}

void UTimerWidget::TimeTick()
{
	CurrentGameTime += 1;
	const float TotalSeconds = CurrentGameTime - GameTargetTime;

	const bool bIsPreGame = TotalSeconds < 0;
	if (bIsPreGame)
	{
		// Для форматирования берем абсолютное значение
		const int32 AbsSeconds = FMath::Abs(TotalSeconds);
		const int32 Minutes = AbsSeconds / 60;
		const int32 Seconds = AbsSeconds % 60;

		// Формируем строку
		const FString Sign = bIsPreGame ? TEXT("-") : TEXT("");
		const FString TimeStr = FString::Printf(TEXT("%s%02d:%02d"), *Sign, Minutes, Seconds);

		TimeText->SetText(FText::FromString(TimeStr));
	}
	else
	{
		TimeText->SetText(FText::FromString(UMD_UIHelper::FormatTime(TotalSeconds)));
	}
}