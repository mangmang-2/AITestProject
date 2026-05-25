// MotionSlotWidget.cpp
#include "MotionSlotWidget.h"
#include "CustomizingMotionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"

// ============================================================
// 초기화
// ============================================================
void UMotionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectButton != nullptr)
	{
		SelectButton->OnClicked.AddDynamic(this, &UMotionSlotWidget::OnSelectButtonClicked);
	}

	if (MoveUpBtn != nullptr)
	{
		MoveUpBtn->OnClicked.AddDynamic(this, &UMotionSlotWidget::OnMoveUpBtnClicked);
	}

	if (MoveDownBtn != nullptr)
	{
		MoveDownBtn->OnClicked.AddDynamic(this, &UMotionSlotWidget::OnMoveDownBtnClicked);
	}
}

void UMotionSlotWidget::InitSlot(int32 InSlotIndex, UCustomizingMotionComponent* InComp)
{
	SlotIndex  = InSlotIndex;
	MotionComp = InComp;
	Refresh();
}

// ============================================================
// Refresh — 슬롯 데이터 & 선택 상태 반영
// ============================================================
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

	const FMotionSlotData& D   = Slots[SlotIndex];
	const bool             bHas = D.IsValid();

	// ── 슬롯 번호 ─────────────────────────────────────────────
	if (SlotLabel != nullptr)
	{
		SlotLabel->SetText(FText::FromString(FString::Printf(TEXT("%d"), SlotIndex + 1)));
		SlotLabel->SetColorAndOpacity(FSlateColor(bIsSelected ? TxtSelHl : TxtMain));
	}

	// ── 모션 이름 ─────────────────────────────────────────────
	if (MotionNameText != nullptr)
	{
		const FText DisplayText = bHas
			? (D.DisplayName.IsEmpty() ? FText::FromString(D.ActionName) : D.DisplayName)
			: FText::FromString(TEXT("클릭해서 모션 설정하기"));

		MotionNameText->SetText(DisplayText);
		MotionNameText->SetColorAndOpacity(FSlateColor(bHas ? TxtMain : TxtEmpty));
	}

	// ── 배경색 (선택 상태 반영) ───────────────────────────────
	if (Border_SlotRow != nullptr)
	{
		Border_SlotRow->SetBrushColor(bIsSelected ? BgRowSel : BgRow);
	}

	if (SelectButton != nullptr)
	{
		SelectButton->SetBackgroundColor(bIsSelected ? BgRowSel : BgRow);
	}
}

// ============================================================
// SetSelected — 선택 상태 변경 후 시각 갱신
// ============================================================
void UMotionSlotWidget::SetSelected(bool bSel)
{
	bIsSelected = bSel;
	Refresh();
}

// ============================================================
// 버튼 핸들러
// ============================================================
void UMotionSlotWidget::OnSelectButtonClicked()
{
	OnSlotClicked.Broadcast();
}

void UMotionSlotWidget::OnMoveUpBtnClicked()
{
	OnMoveRequested.Broadcast(SlotIndex, -1);
}

void UMotionSlotWidget::OnMoveDownBtnClicked()
{
	OnMoveRequested.Broadcast(SlotIndex, 1);
}
