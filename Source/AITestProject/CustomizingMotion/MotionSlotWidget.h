// MotionSlotWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMotionTypes.h"
#include "MotionSlotWidget.generated.h"

class UCustomizingMotionComponent;
class UTextBlock;
class UBorder;
class UDragDropOperation;

// 슬롯 클릭 → SelectSlot 호출용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotClicked);

// 드래그 드롭 → CustomizingMotionWidget::OnSlotReordered 호출용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotDropped, int32, FromSlot, int32, ToSlot);

// ──────────────────────────────────────────────────────────────
// 모션 슬롯 행 위젯 (WBP_MotionSlotWidget 에 reparent)
// 행 전체 클릭 → 선택  /  행 전체 드래그 → 순서 이동
// ──────────────────────────────────────────────────────────────
UCLASS()
class AITESTPROJECT_API UMotionSlotWidget : public UUserWidget
{
	GENERATED_BODY()

	// ── 공개 API ──────────────────────────────────────────────
public:
	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void InitSlot(int32 InSlotIndex, UCustomizingMotionComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void SetSelected(bool bSel);

	// ── 델리게이트 ────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "MotionSlot")
	FOnSlotClicked OnSlotClicked;

	UPROPERTY(BlueprintAssignable, Category = "MotionSlot")
	FOnSlotDropped OnSlotDropped;

	// ── UUserWidget 오버라이드 ─────────────────────────────────
protected:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual void NativeOnDragEnter(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	// ── WBP BindWidget ────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Border_SlotRow = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SlotLabel = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MotionNameText = nullptr;

	// ── 스타일 UPROPERTY (WBP 에디터에서 수정 가능) ───────────
	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor BgRow     = FLinearColor(0.09f, 0.09f, 0.11f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor BgRowSel  = FLinearColor(0.17f, 0.14f, 0.22f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor BgRowHov  = FLinearColor(0.12f, 0.12f, 0.15f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor BgRowDrag = FLinearColor(0.22f, 0.30f, 0.42f, 1.00f);


	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor TxtMain   = FLinearColor(0.85f, 0.85f, 0.88f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor TxtEmpty  = FLinearColor(0.38f, 0.38f, 0.41f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor TxtSelHl  = FLinearColor(0.88f, 0.82f, 1.00f, 1.00f);

	// ── 내부 상태 ─────────────────────────────────────────────
private:
	int32 SlotIndex     = -1;
	bool  bIsSelected   = false;
	bool  bDragDetected = false;

	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;

	/** Border_SlotRow 에 FSlateBrush(RoundedBox + Outline) 를 적용하는 헬퍼 */
	void ApplyRowBrush(const FLinearColor& BgColor);
};
