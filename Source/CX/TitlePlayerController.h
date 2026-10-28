// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

/**
 * 
 */

class UUserWidget;

UCLASS()
class CX_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
	void JoinServer(const FString& InIPAddress);


private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = TitlePlayerController, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = TitlePlayerController, meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidgetInstance;
};
