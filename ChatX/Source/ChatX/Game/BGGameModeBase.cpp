// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BGGameModeBase.h"
#include "Game/BGGameStateBase.h"
#include "Player/BGPlayerController.h"
#include "Player/BGPlayerState.h"
#include "UI/BGUserWidget.h"
#include "EngineUtils.h"

void ABGGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABGPlayerController* BGPlayerController = Cast<ABGPlayerController>(NewPlayer);
	if (IsValid(BGPlayerController) == true)
	{
		AllPlayerControllers.Add(BGPlayerController);
		BGPlayerController->NotificationText = FText::FromString(TEXT("Wating Player..."));
		BGPlayerController->TimerText = FText::FromString(FString::Printf(TEXT("%d"), (int32)LimitTimeAmount));
		BGPlayerController->ClientRPCChangeTurn(false);

		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->PlayerName = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
			BGPlayerController->ChanceText = FText::FromString(FString::Printf(TEXT("%d"), BGPS->GetRemainChanceCnt()));
		}

		ABGGameStateBase* BGGameStateBase = GetGameState<ABGGameStateBase>();
		if (IsValid(BGGameStateBase) == true)
		{
			BGGameStateBase->MulticastRPCBroadcastLoginMessage(BGPS->PlayerName);
		}
	}
}

void ABGGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(StartDelayTimerHandler, this, &ABGGameModeBase::StartGame, 3.f, true);
}

void ABGGameModeBase::PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	bool bIsValidInput = false;
	FString PlayerInput = InChatMessageString;
	FString ResultMessageString = TEXT(" ⇨ ");

	if (IsValidInput(PlayerInput))
	{
		bIsValidInput = true;
		FString GuessResult = EvaluateGuess(ServerNumString, PlayerInput);
		ResultMessageString += GuessResult;

		IncreaseChanceCnt(InChattingPlayerController);
		ChangTurn();
		JudgeInputGame(InChattingPlayerController, GuessResult);
	}
	else
	{
		ResultMessageString += TEXT("OUT (잘못된 입력)");
	}

	ABGPlayerState* BGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
	if (IsValid(BGPS) == true)
	{
		ResultMessageString = BGPS->GetPlayerInfoString() + TEXT(": ") + ResultMessageString;
	}

	ResultMessageString += FString::Printf(TEXT("  ServerNum: %s"), *ServerNumString);
	
	for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It)
	{
		ABGPlayerController* BGPlayerController = *It;
		if (IsValid(BGPlayerController) == true)
		{
			BGPlayerController->ClientRPCPrintChatMessageString(ResultMessageString);

		}
	}
}

FString ABGGameModeBase::GetRandNum()
{
	FString RandNum = "";
	TArray<bool> InputNum;
	InputNum.Init(false, 10);
	while (RandNum.Len() < 3)
	{
		int32 SelectNum = FMath::RandRange(1, 9);
		if (!InputNum[SelectNum])
		{
			RandNum += FString::FromInt(SelectNum);
			InputNum[SelectNum] = true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *RandNum);
	return RandNum;
}

FString ABGGameModeBase::EvaluateGuess(FString ServerNum, FString PlayerNum)
{
	TArray<int> StrikesAndBulls;
	FString Result;
	StrikesAndBulls.Init(0, 2);
	PlayerNum.RemoveAt(0);

	for (int32 i = 0; i < PlayerNum.Len(); i++)
	{
		for (int32 j = 0; j < ServerNum.Len(); j++)
		{
			if (PlayerNum[i] == ServerNum[j])
			{
				if (i == j)
				{
					StrikesAndBulls[0]++;
				}
				else
				{
					StrikesAndBulls[1]++;
				}
			}
		}
	}
	if (StrikesAndBulls[0] == 0 && StrikesAndBulls[1] == 0)
	{
		Result = TEXT("OUT");
	}
	else
	{
		Result = FString::FromInt(StrikesAndBulls[0]) + "S" + FString::FromInt(StrikesAndBulls[1]) + "B";
	}

	return Result;
}

bool ABGGameModeBase::IsValidInput(FString InputValue)
{
	TArray<bool> InputNum;
	InputNum.Init(false, 10);
	if (InputValue[0] != '/' || InputValue.Len() != 4)
	{
		return false;
	}

	for (int32 i = 1; i < InputValue.Len(); i++)
	{
		int32 NumIdx = InputValue[i] - '0';
		if (!InputNum[NumIdx])
		{
			InputNum[NumIdx] = true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void ABGGameModeBase::IncreaseChanceCnt(ABGPlayerController* InChattingPlayerController)
{
	ABGPlayerState* BGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
	if (IsValid(BGPS) == true)
	{
		BGPS->CurrChanceCnt++;
	}
}

void ABGGameModeBase::StartGame()
{
	if (AllPlayerControllers.Num() > 1)
	{
		GetWorldTimerManager().ClearTimer(StartDelayTimerHandler);

		ServerNumString = GetRandNum();

		ChangTurn();
	}
}

void ABGGameModeBase::RestartGame()
{
	CurrTurnIdx = 0;

	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		BGPlayerController->NotificationText = FText::FromString(TEXT("Restart Game!"));
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->CurrChanceCnt = 0;
		}
	}

	StartGame();
}

void ABGGameModeBase::ChangTurn()
{
	ResetTimer();
	JudgeTurn();

	GetWorldTimerManager().SetTimer(TurnTimerHandler, this, &ABGGameModeBase::JudgeTurn, LimitTimeAmount, true);
	GetWorldTimerManager().SetTimer(TimeLimitHandler, this, &ABGGameModeBase::CountTime, 1.f, true);
}

void ABGGameModeBase::JudgeInputGame(ABGPlayerController* InChattingPlayerController, FString GuessResult)
{
	if (GuessResult == TEXT("3S0B"))
	{
		ABGPlayerState* BGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			FString GameResult = BGPS->PlayerName + TEXT(" has won the game!");

			for (const auto& BGPlayerController : AllPlayerControllers)
			{
				BGPlayerController->NotificationText = FText::FromString(GameResult);
				ResetTimer();
				GetWorldTimerManager().SetTimer(NotifyTimerHandler, this, &ABGGameModeBase::RestartGame, 3.f, false);
			}
		}
	}
	else
	{
		if (IsFinishGame())
		{
			for (const auto& BGPlayerController : AllPlayerControllers)
			{
				BGPlayerController->NotificationText = FText::FromString(TEXT("Draw!!"));
				ResetTimer();
				GetWorldTimerManager().SetTimer(NotifyTimerHandler, this, &ABGGameModeBase::RestartGame, 3.f, false);
			}
		}
	}
}

bool ABGGameModeBase::IsFinishGame()
{
	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			if (BGPS->GetRemainChanceCnt()!=0)
			{
				return false;
			}
		}
	}
	return true;
}

void ABGGameModeBase::JudgeTurn()
{
	int32 PreTurnIdx = CurrTurnIdx;
	if(AllPlayerControllers.IsValidIndex(PreTurnIdx))
	{
		auto PC = AllPlayerControllers[PreTurnIdx];
		if(LimitTimeAmount<=0)
		{
			IncreaseChanceCnt(PC);
		}
		LimitTimeAmount = MaxLimitTimeAmount;
		if (IsFinishGame())
		{
			for (const auto& BGPlayerController : AllPlayerControllers)
			{
				BGPlayerController->NotificationText = FText::FromString(TEXT("Draw!!"));
				ResetTimer();
				GetWorldTimerManager().SetTimer(NotifyTimerHandler, this, &ABGGameModeBase::RestartGame, 3.f, false);
				return;
			}
		}
		PC->ClientRPCChangeTurn(false);
	}
	CurrTurnIdx = (CurrTurnIdx + 1) % AllPlayerControllers.Num();

	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		BGPlayerController->ChanceText = FText::FromString(FString::Printf(TEXT("%d"), BGPS->GetRemainChanceCnt()));
		if (BGPlayerController == AllPlayerControllers[CurrTurnIdx])
		{
			AllPlayerControllers[CurrTurnIdx]->NotificationText = FText::FromString(TEXT("It's Your Turn!!"));
			BGPlayerController->ClientRPCChangeTurn(true);
		}
		else
		{
			BGPlayerController->NotificationText = FText::FromString(TEXT("Waitng Your Turn..."));
		}
	}
}

void ABGGameModeBase::CountTime()
{
	--LimitTimeAmount;
	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		BGPlayerController->TimerText = FText::FromString(FString::Printf(TEXT("%d"), (int32)LimitTimeAmount));
	}
}

void ABGGameModeBase::ResetTimer()
{
	GetWorldTimerManager().ClearTimer(TurnTimerHandler);
	GetWorldTimerManager().ClearTimer(TimeLimitHandler);
}
