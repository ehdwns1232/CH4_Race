// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleLayout.generated.h"

/**
 * 
 */

class UButton;
class UEditableText;

UCLASS()
class CX_API UTitleLayout : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UTitleLayout(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TitleWidget, meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TitleWidget, meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LobbyLevelUI, meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UEditableText> ServerIPEditableText;
};
