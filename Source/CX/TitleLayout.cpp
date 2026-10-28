// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleLayout.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TitlePlayerController.h"


UTitleLayout::UTitleLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UTitleLayout::NativeConstruct()
{
	PlayButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnPlayButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UTitleLayout::OnPlayButtonClicked()
{
	ATitlePlayerController* TitlePlayerController = GetOwningPlayer<ATitlePlayerController>();
	if (IsValid(TitlePlayerController))
	{
		FText ServerIP = ServerIPEditableText->GetText();
		TitlePlayerController->JoinServer(ServerIP.ToString());
	}
}


void UTitleLayout::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
