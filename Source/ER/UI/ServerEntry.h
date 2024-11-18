#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "ServerEntry.generated.h"

UCLASS(BlueprintType)
class ER_API UServerEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
private:
	FString ServerID;
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ServerNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MapNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ServerPingText;
	
	UPROPERTY(meta = (BindWidget))
	class UBorder* EntryBorder;
	
	FString GetConnectionInfo() const;
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBorderColor(const FLinearColor& NewColor);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
