// CustomizingMotionWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMotionTypes.h"
#include "MotionSlotWidget.h"
#include "MotionListEntryWidget.h"
#include "MotionListWidget.h"
#include "CustomizingMotionWidget.generated.h"

class UCustomizingMotionComponent;
class UVerticalBox;
class UButton;
class UTextBlock;
class UScrollBox;
class UCanvasPanel;
class USizeBox;

// ── 메인창 → 외부 델리게이트 ────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCustomizingApplyRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCustomizingCloseRequested);

// ──────────────────────────────────────────────────────────────
// 메인 커스터마이징 모션 위젯
// ──────────────────────────────────────────────────────────────
UCLASS()
class AITESTPROJECT_API UCustomizingMotionWidget : public UUserWidget
{
	GENERATED_BODY()

	// ── 공개 API ──────────────────────────────────────────────
public:
	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void InitWidget(UCustomizingMotionComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void RefreshSlots();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void RefreshPresetList();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void OnSavePresetClicked(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void OnLoadPresetClicked(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void OnDeletePresetClicked(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void OnPlayClicked();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotionUI")
	void OnStopClicked();

	// ── UUserWidget 오버라이드 ─────────────────────────────────
protected:
	virtual bool Initialize()      override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;

	// ── BP에서 할당 가능한 서브클래스 ──────────────────────────
	UPROPERTY(EditAnywhere, Category = "CustomizingMotionUI")
	TSubclassOf<UMotionSlotWidget> SlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "CustomizingMotionUI")
	TSubclassOf<UMotionListEntryWidget> ListEntryWidgetClass;

	UPROPERTY(EditAnywhere, Category = "CustomizingMotionUI")
	TSubclassOf<UMotionListWidget> ListWidgetClass;

	// ── WBP BindWidget — 프리셋 / 슬롯 컨테이너 ──────────────
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PresetListScroll = nullptr;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* SlotRowsBox = nullptr;

	// ── 델리게이트 ────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "CustomizingMotionUI")
	FOnCustomizingApplyRequested OnApplyRequested;

	UPROPERTY(BlueprintAssignable, Category = "CustomizingMotionUI")
	FOnCustomizingCloseRequested OnCloseRequested;

	// ── WBP BindWidget — 타이틀 ───────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseBtn = nullptr;

	// ── WBP BindWidget — 기능 버튼 ────────────────────────────
	UPROPERTY(meta = (BindWidget))
	UButton* BtnSavePreset = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnReset = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* CircleBtn = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* BtnApply = nullptr;

	// ── WBP BindWidget — 재생 상태 텍스트 ─────────────────────
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CircleStateTxt = nullptr;

	// ── 스타일 UPROPERTY (WBP 에디터에서 수정 가능) ───────────
	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor PresetBtnBgColor       = FLinearColor(0.12f, 0.12f, 0.16f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor PresetTxtColor         = FLinearColor(0.85f, 0.85f, 0.88f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor PresetTxtEmptyColor    = FLinearColor(0.45f, 0.45f, 0.48f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor PresetDividerColor     = FLinearColor(0.05f, 0.05f, 0.06f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor PlayingIndicatorColor  = FLinearColor(0.20f, 0.85f, 0.35f, 1.00f);

	UPROPERTY(EditAnywhere, Category = "Style")
	FLinearColor StoppedIndicatorColor  = FLinearColor(0.45f, 0.45f, 0.48f, 1.00f);

	// ── 내부 상태 ─────────────────────────────────────────────
private:
	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;

	UPROPERTY()
	UMotionListWidget* MotionListWidget = nullptr;

	UPROPERTY()
	TArray<UMotionSlotWidget*> SlotWidgets;

	int32 SelectedSlotIndex = -1;

	void ShowListWindow(bool bShow);
	void SelectSlot(int32 Index);

	// ── 슬롯 선택 (S0~S4) ────────────────────────────────────
	UFUNCTION() void S0();
	UFUNCTION() void S1();
	UFUNCTION() void S2();
	UFUNCTION() void S3();
	UFUNCTION() void S4();

	// ── 프리셋 로드 (P0~P9) ──────────────────────────────────
	void LoadPresetAndRefresh(int32 Idx);

	UFUNCTION() void P0();
	UFUNCTION() void P1();
	UFUNCTION() void P2();
	UFUNCTION() void P3();
	UFUNCTION() void P4();
	UFUNCTION() void P5();
	UFUNCTION() void P6();
	UFUNCTION() void P7();
	UFUNCTION() void P8();
	UFUNCTION() void P9();

	// ── 버튼 핸들러 ──────────────────────────────────────────
	UFUNCTION() void OnSavePresetBtnClicked();
	UFUNCTION() void OnResetAllClicked();
	UFUNCTION() void OnCirclePlayClicked();
	UFUNCTION() void OnApplyBtnClicked();
	UFUNCTION() void OnCloseBtnClicked();

	// ── MotionSlotWidget 델리게이트 수신 ─────────────────────
	UFUNCTION() void OnSlotMoveRequested(int32 SlotIdx, int32 Direction);

	// ── MotionListWidget 델리게이트 수신 ─────────────────────
	UFUNCTION() void OnListCloseRequested();
	UFUNCTION() void OnMotionApplied(int32 SlotIndex, FMotionSlotData MotionData);

	// ── 컴포넌트 델리게이트 ──────────────────────────────────
	UFUNCTION() void OnSlotsChanged(const TArray<FMotionSlotData>& Slots);
	UFUNCTION() void OnPlayStateChanged(bool bPlaying);

	// ── 기존 호환용 ───────────────────────────────────────────
	void AssignMotion(int32 MotionIndex);

	UFUNCTION() void OnClearSlotClicked();
	UFUNCTION() void OnSaveClicked();

	void PopulateTestMotions();
};
