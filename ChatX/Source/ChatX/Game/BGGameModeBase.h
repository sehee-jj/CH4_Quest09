// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BGGameModeBase.generated.h"

class ABGPlayerController;

/**
 * 
 */
UCLASS()
class CHATX_API ABGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	void OnPostLogin(AController* NewPlayer) override;

	virtual void BeginPlay() override;

	void PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	FString GetRandNum();

	FString EvaluateGuess(FString ServerNum, FString PlayerNum);

	bool IsValidInput(FString InputValue);

	void IncreaseChanceCnt(ABGPlayerController* InChattingPlayerController);
	
	UFUNCTION()
	void StartGame();

	void RestartGame();
	void ChangTurn();
	
	void JudgeInputGame(ABGPlayerController* InChattingPlayerController, FString GuessResult);
	bool IsFinishGame();

	UFUNCTION()
	void JudgeTurn();

	UFUNCTION()
	void CountTime();

	void ResetTimer();

protected:
	FString ServerNumString;

	TArray<TObjectPtr<ABGPlayerController>> AllPlayerControllers;

	int32 CurrTurnIdx = 0;

	float LimitTimeAmount = 15;
	float MaxLimitTimeAmount = 15;

	FTimerHandle NotifyTimerHandler;
	FTimerHandle TurnTimerHandler;
	FTimerHandle TimeLimitHandler;
	FTimerHandle StartDelayTimerHandler;
};
