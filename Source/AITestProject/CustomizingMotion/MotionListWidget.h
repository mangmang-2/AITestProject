// MotionListWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMotionTypes.h"
#include "MotionListWidget.generated.h"

class UCustomizingMotionComponent;
class UButton;
class UTextBlock;
class UScrollBox;
class UBorder;
class UEditableTextBox;

// ── 리스트창 → 메인 위젯 델리게이트 ─────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMotionListCloseRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMotionApplied, int32, SlotIndex, FMotionSlotData, MotionData);

// ──────────────────────────────────────────────────────────────
// 모션 리스트 창 위젯 (별도 분리)
// ──────────────────────────────────────────────────────────────
UCLASS()
class AITESTPROJECT_API UMotionListWidget : public UUserWidget
{
	GENERATED_BODY()

	// ── 공개 API ──────────────────────────────────────────────
public:
	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void InitWidget(UCustomizingMotionComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void SetTargetSlot(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void RefreshMotionList();

	// ── 델리게이트 ────────────────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "MotionList")
	FOnMotionListCloseRequested OnCloseRequested;

	UPROPERTY(BlueprintAssignable, Category = "MotionList")
	FOnMotionApplied OnMotionApplied;

	// ── UUserWidget 오버라이드 ─────────────────────────────────
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;

	// ── WBP BindWidget ────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	UButton* TabCostumeBtn = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* TabGestureBtn = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TabCostumeTxt = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TabGestureTxt = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseListBtn = nullptr;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* MotionBodyScroll = nullptr;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnApplyList = nullptr;

	// ── 검색 / 필터 (선택적) ─────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* SearchBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* StarFilterBtn = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StarFilterTxt = nullptr;

	// ── 내부 상태 ─────────────────────────────────────────────
private:
	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;

	int32 TargetSlotIndex    = -1;
	int32 PendingMotionIndex = -1;   // Av[] 실제 인덱스
	int32 ActiveTab          =  0;

	FString CurrentSearchText;
	bool    bShowBookmarksOnly = false;

	// 화면에 표시된 행 EntryIdx → Av[] 실제 인덱스 매핑
	TArray<int32> DisplayedIndices;

	void SelectListMotion(int32 EntryIdx);
	void ToggleBookmark(int32 EntryIdx);

	// ── 탭 / 닫기 / 적용 핸들러 ──────────────────────────────
	UFUNCTION() void OnTabCostumeClicked();
	UFUNCTION() void OnTabGestureClicked();
	UFUNCTION() void OnCloseListClicked();
	UFUNCTION() void OnApplyMotionClicked();

	// ── 검색 / 즐겨찾기 필터 핸들러 ─────────────────────────
	UFUNCTION() void OnSearchTextChanged(const FText& Text);
	UFUNCTION() void OnStarFilterBtnClicked();

	// ── 모션 행 선택 (L0~L9) ─────────────────────────────────
	UFUNCTION() void L0();
	UFUNCTION() void L1();
	UFUNCTION() void L2();
	UFUNCTION() void L3();
	UFUNCTION() void L4();
	UFUNCTION() void L5();
	UFUNCTION() void L6();
	UFUNCTION() void L7();
	UFUNCTION() void L8();
	UFUNCTION() void L9();

	// ── 북마크 (B0~B9) ───────────────────────────────────────
	UFUNCTION() void B0();
	UFUNCTION() void B1();
	UFUNCTION() void B2();
	UFUNCTION() void B3();
	UFUNCTION() void B4();
	UFUNCTION() void B5();
	UFUNCTION() void B6();
	UFUNCTION() void B7();
	UFUNCTION() void B8();
	UFUNCTION() void B9();
};
