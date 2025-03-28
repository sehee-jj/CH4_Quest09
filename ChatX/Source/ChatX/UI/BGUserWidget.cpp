// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BGUserWidget.h"
#include "Components/EditableTextBox.h"
#include "Player/BGPlayerController.h"

void UBGUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UBGUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UBGUserWidget::ChangeTurnUI(bool bIsOn)
{
	EditableTextBox_ChatInput->SetIsEnabled(bIsOn);
}

void UBGUserWidget::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* OwningPlayerController = GetOwningPlayer();
		if (IsValid(OwningPlayerController) == true)
		{
			ABGPlayerController* OwningBGPlayerController = Cast<ABGPlayerController>(OwningPlayerController);
			if (IsValid(OwningBGPlayerController) == true)
			{
				OwningBGPlayerController->SetChatMessageString(Text.ToString());
				EditableTextBox_ChatInput->SetText(FText());
			}
		}
	}
}
