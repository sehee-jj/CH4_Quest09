// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BGPlayerState.h"
#include "Net/UnrealNetwork.h"

ABGPlayerState::ABGPlayerState()
	: PlayerName(TEXT("None")),
	CurrChanceCnt(0),
	MaxChanceCnt(3)
{
	bReplicates = true;
}

void ABGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerName);
	DOREPLIFETIME(ThisClass, CurrChanceCnt);
}

FString ABGPlayerState::GetPlayerInfoString()
{
	FString PlayerInfoString = PlayerName + TEXT("(") + FString::FromInt(CurrChanceCnt) + TEXT("/") + FString::FromInt(MaxChanceCnt) + TEXT(")");

	return PlayerInfoString;
}
