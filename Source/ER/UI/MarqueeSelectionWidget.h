// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MarqueeSelectionWidget.generated.h"

/**
 * 
 */

UCLASS()
class ER_API UMarqueeSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateMarquee(const FVector2D& StartPos, const FVector2D& CurrentPos);

protected:
	UPROPERTY(meta = (BindWidget))
	class UBorder* SelectionBorder;

	virtual void NativeConstruct() override;
};