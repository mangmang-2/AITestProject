// MotionSlotWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMotionTypes.h"
#include "MotionSlotWidget.generated.h"

class UCustomizingMotionComponent;
class UTextBlock;
class UButton;
class UBorder;

// 슬롯 클릭 → SelectSlot 호출용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotClicked);

// 슬롯 순서 이동 → CustomizingMotionWidget::OnSlotMoveRequested 호출용
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotMoveRequested, int32, SlotIdx, int32, Direction);

// ──────────────────────────────────────────────────────────────
// 모션 슬롯 행 위젯 (WBP_MotionSlotWidget 에 reparent)
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

	/** 선택 상태 변경 후 시각 갱신 */
	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void SetSelected(bool bSel);

	// ── 델리게이트 ────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "MotionSlot")
	FOnSlotClicked OnSlotClicked;

	UPROPERTY(BlueprintAssignable, Category = "MotionSlot")
	FOnSlotMoveRequested OnMoveRequested;

	// ── UUserWidget 오버라이드 ─────────────────────────────────
protected:
	virtual void NativeConstruct() override;

	// ── WBP BindWidget ────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Border_SlotRow = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MoveUpBtn = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MoveDownBtn = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* SelectButton = nullptr;

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
	FLinearColor TxtMain   = FLinearColor(0.85f, 0.85f, 0.88f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor TxtEmpty  = FLinearColor(0.38f, 0.38f, 0.41f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor TxtSelHl  = FLinearColor(0.88f, 0.82f, 1.00f, 1.00f);

	// ── 내부 상태 ─────────────────────────────────────────────
private:
	int32 SlotIndex   = -1;
	bool  bIsSelected = false;

	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;

	UFUNCTION() void OnSelectButtonClicked();
	UFUNCTION() void OnMoveUpBtnClicked();
	UFUNCTION() void OnMoveDownBtnClicked();
};
