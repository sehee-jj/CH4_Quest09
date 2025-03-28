// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BGGameModeBase.h"
#include "Game/BGGameStateBase.h"
#include "Player/BGPlayerController.h"
#include "Player/BGPlayerState.h"
#include "EngineUtils.h"

void ABGGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABGPlayerController* BGPlayerController = Cast<ABGPlayerController>(NewPlayer);
	if (IsValid(BGPlayerController) == true)
	{
		BGPlayerController->NotificationText = FText::FromString(TEXT("Connected to Game server!"));
		AllPlayerControllers.Add(BGPlayerController);

		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->PlayerName = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
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

	ServerNumString = GetRandNum();
}

void ABGGameModeBase::PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	bool bIsValidInput = false;
	FString PlayerInput = InChatMessageString;
	/*TArray<FString> PlayerInput;
	InChatMessageString.ParseIntoArray(PlayerInput, TEXT(" "));*/
	FString ResultMessageString = TEXT(" ⇨ ");

	if (IsValidInput(PlayerInput))
	{
		bIsValidInput = true;
		FString GuessResult = EvaluateGuess(ServerNumString, PlayerInput);
		ResultMessageString += GuessResult;

		IncreaseChanceCnt(InChattingPlayerController);
		JudgeGame(InChattingPlayerController, GuessResult);
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
	int32 Num = 0;
	for (int32 i = 0; i < 3; i++)
	{
		Num += FMath::Pow(10.f, i) * FMath::RandRange(1, 9);
	}
	return FString::FromInt(Num);
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

void ABGGameModeBase::RestartGame()
{
	ServerNumString = GetRandNum();

	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		BGPlayerController->NotificationText = FText::FromString(TEXT("Restart Game!"));
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->CurrChanceCnt = 0;
		}
	}


}

void ABGGameModeBase::JudgeGame(ABGPlayerController* InChattingPlayerController, FString GuessResult)
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
				RestartGame();
			}
		}
	}
	else
	{
		bool bIsDraw = false;
		for (const auto& BGPlayerController : AllPlayerControllers)
		{
			ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
			if (IsValid(BGPS) == true)
			{
				if (BGPS->CurrChanceCnt > BGPS->MaxChanceCnt)
				{
					bIsDraw = true;
					break;
				}
			}
		}

		if (bIsDraw)
		{
			for (const auto& BGPlayerController : AllPlayerControllers)
			{
				BGPlayerController->NotificationText = FText::FromString(TEXT("Draw!!"));
				GetWorldTimerManager().SetTimer(NotifyTimer, this, &ABGGameModeBase::RestartGame, 3.f, false);
			}
		}
	}
}
