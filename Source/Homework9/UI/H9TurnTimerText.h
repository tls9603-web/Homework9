// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "H9TurnTimerText.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK9_API UH9TurnTimerText : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 남은 시간을 텍스트로 반환하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Timer")
	FText GetRemainingTimeText() const;

	// 현재 턴인지 확인하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Timer")
	bool IsMyTurn() const;

	// 남은 시간을 0~1 사이의 값으로 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Timer")
	float GetRemainingTimePercent() const;

protected:
	// 턴 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Timer")
	float MaxTurnDuration;

private:
	class AH9PlayerState* GetLocalPlayerState() const;
};
