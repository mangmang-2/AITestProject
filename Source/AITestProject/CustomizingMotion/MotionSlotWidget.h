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

// 슬롯 클릭 → CustomizingMotionWidget 에서 SelectSlot() 호출용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotClicked);

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

	/** 선택 상태 변경 (CustomizingMotionWidget 이 호출) */
	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void SetSelected(bool bSel);

	// ── 델리게이트 ────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "MotionSlot")
	FOnSlotClicked OnSlotClicked;

	// ── UUserWidget 오버라이드 ─────────────────────────────────
protected:
	virtual void NativeConstruct() override;

	// ── WBP BindWidget ────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Border_SlotRow = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Border_Num = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* SelectButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SlotLabel = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MotionNameText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ClearButton = nullptr;

	// ── 내부 상태 ─────────────────────────────────────────────
private:
	int32 SlotIndex  = -1;
	bool  bIsSelected = false;

	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;

	UFUNCTION() void OnSelectButtonClicked();
	UFUNCTION() void OnClearClicked();
};
