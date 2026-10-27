#include "ChatUI.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "CX/CXPlayerController.h"

void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommited) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommited);
	}
}

void UChatUI::NativeDestruct()
{
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommited) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &UChatUI::OnChatInputTextCommited);
	}

	Super::NativeDestruct();
}

void UChatUI::OnNewChangeMessage(const FString& Sender, const FString& Message)
{
	FString NewMessage = Sender + TEXT(": ") + Message;
	FString OldMessage = MultiLineEditableBox_ChatLog->GetText().ToString();
	OldMessage += '\n';
	FText TextMessage = FText::FromString(OldMessage += NewMessage);

	MultiLineEditableBox_ChatLog->SetText(TextMessage);
}

void UChatUI::OnChatInputTextCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		FString ChatMessage = Text.ToString();
		EditableTextBox_ChatInput->SetText(FText::GetEmpty());
		if (ACXPlayerController* PC = GetOwningPlayer<ACXPlayerController>())
		{
			PC->CTSChatMessage(ChatMessage);
		}
	}
}
