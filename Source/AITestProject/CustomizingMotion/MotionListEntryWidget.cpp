// MotionListEntryWidget.cpp
#include "MotionListEntryWidget.h"
#include "CustomizingMotionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMotionListEntryWidget::Init(const FMotionSlotData& InData, UCustomizingMotionComponent* InComp)
{
	MotionData = InData;
	MotionComp = InComp;

	if (EntryNameText != nullptr)
	{
		const FText DisplayText = MotionData.DisplayName.IsEmpty()
			? FText::FromString(MotionData.ActionName)
			: MotionData.DisplayName;
		EntryNameText->SetText(DisplayText);
	}
}

void UMotionListEntryWidget::OnAssignToSlot(int32 TargetSlotIndex)
{
	if (MotionComp != nullptr)
	{
		MotionComp->SetSlot(TargetSlotIndex, MotionData);
	}
}

void UMotionListEntryWidget::OnPreview()
{
	if (MotionComp != nullptr)
	{
		MotionComp->PlaySingleSlot(0, false);
	}
}
