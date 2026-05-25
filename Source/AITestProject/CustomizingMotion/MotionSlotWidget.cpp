// MotionSlotWidget.cpp
#include "MotionSlotWidget.h"
#include "CustomizingMotionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"

// ============================================================
// 슬롯 위젯 색상 상수
// ============================================================
namespace MS
{
	const FLinearColor BgRow    (0.09f, 0.09f, 0.11f, 1.00f);
	const FLinearColor BgRowSel (0.17f, 0.14f, 0.22f, 1.00f);
	const FLinearColor BgNum    (0.11f, 0.11f, 0.15f, 1.00f);
	const FLinearColor BgNumSel (0.24f, 0.18f, 0.34f, 1.00f);
	const FLinearColor TxtMain  (0.85f, 0.85f, 0.88f, 1.00f);
	const FLinearColor TxtEmpty (0.38f, 0.38f, 0.41f, 1.00f);
	const FLinearColor TxtSelHl (0.88f, 0.82f, 1.00f, 1.00f);
}

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
		SlotLabel->SetColorAndOpacity(FSlateColor(bIsSelected ? MS::TxtSelHl : MS::TxtMain));
	}

	// ── 모션 이름 ─────────────────────────────────────────────
	if (MotionNameText != nullptr)
	{
		const FText DisplayText = bHas
			? (D.DisplayName.IsEmpty() ? FText::FromString(D.ActionName) : D.DisplayName)
			: FText::FromString(TEXT("클릭해서 모션 설정하기"));

		MotionNameText->SetText(DisplayText);
		MotionNameText->SetColorAndOpacity(FSlateColor(bHas ? MS::TxtMain : MS::TxtEmpty));
	}

	// ── 배경색 (선택 상태 반영) ───────────────────────────────
	if (Border_SlotRow != nullptr)
	{
		Border_SlotRow->SetBrushColor(bIsSelected ? MS::BgRowSel : MS::BgRow);
	}

	if (Border_Num != nullptr)
	{
		Border_Num->SetBrushColor(bIsSelected ? MS::BgNumSel : MS::BgNum);
	}

	if (SelectButton != nullptr)
	{
		SelectButton->SetBackgroundColor(bIsSelected ? MS::BgRowSel : MS::BgRow);
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

void UMotionSlotWidget::OnClearClicked()
{
	if (MotionComp != nullptr)
	{
		MotionComp->ClearSlot(SlotIndex);
	}
}
