#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatUI.generated.h"

UCLASS()
class CX_API UChatUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void OnNewChangeMessage(const FString& Sender, const FString& Message);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnChatInputTextCommited(const FText& Text, ETextCommit::Type CommitMethod);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox> EditableTextBox_ChatInput;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UMultiLineEditableTextBox> MultiLineEditableBox_ChatLog;
};
