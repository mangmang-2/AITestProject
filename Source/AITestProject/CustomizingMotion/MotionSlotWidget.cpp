// MotionSlotWidget.cpp
#include "MotionSlotWidget.h"
#include "MotionSlotDragDropOperation.h"
#include "CustomizingMotionComponent.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Styling/SlateBrush.h"

// ============================================================
// 초기화
// ============================================================
void UMotionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 행 전체가 클릭/드래그 영역 — SelectButton 없음
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

	const FMotionSlotData& D    = Slots[SlotIndex];
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

	// ── 배경색 + 아웃라인 ─────────────────────────────────────
	ApplyRowBrush(bIsSelected ? BgRowSel : BgRow);
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
// 헬퍼 — Border_SlotRow 브러시(배경 + 아웃라인) 적용
// ============================================================
void UMotionSlotWidget::ApplyRowBrush(const FLinearColor& BgColor)
{
	if (Border_SlotRow == nullptr)
	{
		return;
	}

	// WBP 에디터에서 설정한 브러시(DrawAs, CornerRadii, 아웃라인 등)를 그대로 읽어
	// TintColor(배경색)만 동적으로 교체 — 나머지 설정은 에디터에서 관리
	// UBorder에는 GetBrush()가 없으므로 Background 프로퍼티 직접 접근
	FSlateBrush Brush = Border_SlotRow->Background;
	Brush.TintColor   = FSlateColor(BgColor);
	Border_SlotRow->SetBrush(Brush);
}

// ============================================================
// 클릭 & 드래그 — 행 전체가 클릭/드래그 영역
// ============================================================
FReply UMotionSlotWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	bDragDetected = false;

	TSharedPtr<SWidget> SlateWidget = GetCachedWidget();
	if (SlateWidget.IsValid())
	{
		// DetectDrag: 마우스가 임계값 이상 이동하면 NativeOnDragDetected 호출
		return FReply::Handled().DetectDrag(SlateWidget.ToSharedRef(), EKeys::LeftMouseButton);
	}

	return FReply::Handled();
}

FReply UMotionSlotWidget::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	// 드래그가 시작되지 않았으면 클릭으로 처리 → 모션 선택창 열기
	if (bDragDetected == false)
	{
		OnSlotClicked.Broadcast();
	}

	return FReply::Handled();
}

void UMotionSlotWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	bDragDetected = true;

	UMotionSlotDragDropOperation* DragOp = NewObject<UMotionSlotDragDropOperation>(this);
	DragOp->DraggedSlotIndex = SlotIndex;

	// 드래그 비주얼: 같은 WBP 클래스로 새 인스턴스를 생성 — 실제 슬롯 행과 동일한 모양
	// (this 자체를 사용하면 VerticalBox에서 꺼내져 레이아웃이 깨짐)
	UMotionSlotWidget* DragVisual = CreateWidget<UMotionSlotWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual != nullptr)
	{
		DragVisual->InitSlot(SlotIndex, MotionComp);
		DragVisual->SetRenderOpacity(0.75f);  // 반투명으로 드래그 중임을 표시
		DragOp->DefaultDragVisual = DragVisual;
	}

	DragOp->Pivot = EDragPivot::MouseDown;
	OutOperation  = DragOp;
}

bool UMotionSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UMotionSlotDragDropOperation* DragOp = Cast<UMotionSlotDragDropOperation>(InOperation);
	if (DragOp == nullptr)
	{
		return false;
	}
	if (DragOp->DraggedSlotIndex == SlotIndex)
	{
		return false;
	}

	OnSlotDropped.Broadcast(DragOp->DraggedSlotIndex, SlotIndex);
	return true;
}

void UMotionSlotWidget::NativeOnDragEnter(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (Cast<UMotionSlotDragDropOperation>(InOperation) == nullptr)
	{
		return;
	}

	ApplyRowBrush(BgRowDrag);
}

void UMotionSlotWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	ApplyRowBrush(bIsSelected ? BgRowSel : BgRow);
}
