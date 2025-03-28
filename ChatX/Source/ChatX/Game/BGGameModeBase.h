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

	void RestartGame();
	
	void JudgeGame(ABGPlayerController* InChattingPlayerController, FString GuessResult);

protected:
	FString ServerNumString;

	TArray<TObjectPtr<ABGPlayerController>> AllPlayerControllers;

	TObjectPtr<ABGPlayerController> CurrTurnPlayer;

	FTimerHandle NotifyTimer;
	FTimerHandle TurnTiemr;
};
