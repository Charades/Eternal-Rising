#include "MarqueeSelectionWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

void UMarqueeSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectionBorder)
	{
		// Set initial visibility to hidden
		SelectionBorder->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMarqueeSelectionWidget::UpdateMarquee(const FVector2D& StartPos, const FVector2D& CurrentPos)
{
	if (!SelectionBorder) return;

	// Get the CanvasPanelSlot dynamically
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SelectionBorder->Slot);
	if (!CanvasSlot) return;

	// Calculate the rectangle's position and size
	FVector2D TopLeft(FMath::Min(StartPos.X, CurrentPos.X), FMath::Min(StartPos.Y, CurrentPos.Y));
	FVector2D Size(FMath::Abs(CurrentPos.X - StartPos.X), FMath::Abs(CurrentPos.Y - StartPos.Y));

	// Update position and size
	CanvasSlot->SetPosition(TopLeft);
	CanvasSlot->SetSize(Size);

	// Make the border visible
	SelectionBorder->SetVisibility(ESlateVisibility::Visible);
}