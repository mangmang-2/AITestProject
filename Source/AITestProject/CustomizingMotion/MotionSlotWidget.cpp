// MotionSlotWidget.cpp
#include "MotionSlotWidget.h"
#include "CustomizingMotionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMotionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ClearButton != nullptr)
	{
		ClearButton->OnClicked.AddDynamic(this, &UMotionSlotWidget::OnClearClicked);
	}
}

void UMotionSlotWidget::InitSlot(int32 InSlotIndex, UCustomizingMotionComponent* InComp)
{
	SlotIndex  = InSlotIndex;
	MotionComp = InComp;
	Refresh();
}

void UMotionSlotWidget::Refresh()
{
	if (MotionComp == nullptr)
	{
		return;
	}

	const TArray<FMotionSlotData>& Slots = MotionComp->GetSlots();

	if (Slots.IsValidIndex(SlotIndex) == false)
	{
		return;
	}

	const FMotionSlotData& D = Slots[SlotIndex];

	if (SlotLabel != nullptr)
	{
		SlotLabel->SetText(FText::FromString(FString::Printf(TEXT("Slot %d"), SlotIndex + 1)));
	}

	if (MotionNameText != nullptr)
	{
		const FText DisplayText = D.IsValid()
			? (D.DisplayName.IsEmpty() ? FText::FromString(D.ActionName) : D.DisplayName)
			: FText::FromString(TEXT("(Empty)"));
		MotionNameText->SetText(DisplayText);
	}
}

void UMotionSlotWidget::OnClearClicked()
{
	if (MotionComp != nullptr)
	{
		MotionComp->ClearSlot(SlotIndex);
	}
}
