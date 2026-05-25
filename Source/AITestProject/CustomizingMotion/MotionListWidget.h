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

	// 창 위치 설정 (내부 ViewportPos 와 SetPositionInViewport 동기화)
	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void SetWindowPosition(FVector2D Pos);

	// ── UUserWidget 오버라이드 ─────────────────────────────────
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct()  override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 터널 단계 (루트→리프) — 자식보다 먼저 호출 보장
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	// ── WBP BindWidget ────────────────────────────────────────
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Border_TitleBar = nullptr;

	// ── WBP BindWidget — 원래 것들 ───────────────────────────
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

	// ── Style ─────────────────────────────────────────────────
	/** 드래그 감지 영역 높이 (논리 픽셀, 타이틀 바 높이와 맞출 것) */
	UPROPERTY(EditAnywhere, Category = "Style")
	float TitleBarHeight = 40.f;

	// ── 내부 상태 ─────────────────────────────────────────────
private:
	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;

	// ── 드래그 이동 ──────────────────────────────────────────
	bool      bIsDragging      = false;
	FVector2D DragLastMousePos = FVector2D::ZeroVector;
	FVector2D ViewportPos      = FVector2D::ZeroVector;  // 현재 논리 픽셀 위치

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
