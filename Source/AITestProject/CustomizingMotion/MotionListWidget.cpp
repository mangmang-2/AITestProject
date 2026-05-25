// MotionListWidget.cpp
#include "MotionListWidget.h"
#include "CustomizingMotionComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/EditableTextBox.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"

// ============================================================
// 색상 상수
// ============================================================
namespace ML
{
	const FLinearColor BgRow    (0.09f, 0.09f, 0.11f, 1.00f);
	const FLinearColor BgRowSel (0.17f, 0.14f, 0.22f, 1.00f);
	const FLinearColor BgTabOn  (0.14f, 0.14f, 0.18f, 1.00f);
	const FLinearColor BgTabOff (0.08f, 0.08f, 0.10f, 1.00f);
	const FLinearColor BgStar   (0.09f, 0.09f, 0.11f, 1.00f);
	const FLinearColor BgStarOn (0.20f, 0.16f, 0.06f, 1.00f);

	const FLinearColor TxtMain  (0.85f, 0.85f, 0.88f, 1.00f);
	const FLinearColor TxtDim   (0.45f, 0.45f, 0.48f, 1.00f);
	const FLinearColor TxtTitle (0.92f, 0.92f, 0.95f, 1.00f);
	const FLinearColor TxtStar  (0.92f, 0.78f, 0.12f, 1.00f);
	const FLinearColor TxtSelHl (0.88f, 0.82f, 1.00f, 1.00f);
}

// ============================================================
// SetWindowPosition — 외부(ShowListWindow)에서 초기 위치 지정 시 사용
// ============================================================
void UMotionListWidget::SetWindowPosition(FVector2D Pos)
{
	ViewportPos = Pos;
	SetPositionInViewport(ViewportPos, false);
}

// ============================================================
// 타이틀 바 드래그 이동
// ============================================================
// NativeOnPreviewMouseButtonDown: 터널 단계(루트→리프) — 자식보다 먼저 호출됨
// NativeOnMouseButtonDown(버블 단계)는 자식이 이벤트를 소비하면 호출 안 됨 → Preview 사용
FReply UMotionListWidget::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	float MouseX = 0.f, MouseY = 0.f;
	PC->GetMousePosition(MouseX, MouseY);

	const float     DPIScale     = UWidgetLayoutLibrary::GetViewportScale(this);
	const FVector2D ClickLogical = FVector2D(MouseX, MouseY) / (DPIScale > 0.f ? DPIScale : 1.f);

	// 타이틀 바 Y 범위 밖이면 패스스루 (탭/스크롤/버튼 정상 동작)
	const float RelativeY = ClickLogical.Y - ViewportPos.Y;
	if (RelativeY < 0.f || RelativeY > TitleBarHeight)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// CloseListBtn 위에 있으면 버튼 클릭을 허용 (드래그 시작 안 함)
	if (CloseListBtn != nullptr)
	{
		const FGeometry& BtnGeo  = CloseListBtn->GetCachedGeometry();
		const FVector2D  BtnSize = BtnGeo.GetLocalSize();
		if (BtnSize.X > 0.f && BtnSize.Y > 0.f)
		{
			const FVector2D BtnLocal = BtnGeo.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
			if (BtnLocal.X >= 0.f && BtnLocal.X <= BtnSize.X &&
				BtnLocal.Y >= 0.f && BtnLocal.Y <= BtnSize.Y)
			{
				return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
			}
		}
	}

	// 타이틀 바 드래그 시작
	bIsDragging      = true;
	DragLastMousePos = FVector2D(MouseX, MouseY);
	return FReply::Handled();
}

void UMotionListWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsDragging)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
		bIsDragging = false;
		return;
	}

	// 마우스 버튼이 떼어졌으면 드래그 종료 (위젯 밖에서 떼는 경우 대응)
	// FReply::Handled()로 소비된 이벤트는 UPlayerInput에 전달되지 않으므로
	// Slate 레이어의 실제 버튼 상태를 직접 확인
	if (!FSlateApplication::IsInitialized() ||
		!FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton))
	{
		bIsDragging = false;
		return;
	}

	// GetMousePosition → 물리 픽셀. DPI 스케일로 나눠 논리 픽셀 델타로 변환
	float MouseX = 0.f, MouseY = 0.f;
	PC->GetMousePosition(MouseX, MouseY);

	const FVector2D CurrentPos(MouseX, MouseY);
	const FVector2D Delta    = CurrentPos - DragLastMousePos;
	DragLastMousePos         = CurrentPos;

	const float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	ViewportPos         += (DPIScale > 0.f) ? (Delta / DPIScale) : Delta;
	SetPositionInViewport(ViewportPos, false);
}

// ============================================================
// NativeConstruct / NativeDestruct
// ============================================================
void UMotionListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	if (TabCostumeBtn != nullptr)
	{
		TabCostumeBtn->OnClicked.AddDynamic(this, &UMotionListWidget::OnTabCostumeClicked);
	}
	if (TabGestureBtn != nullptr)
	{
		TabGestureBtn->OnClicked.AddDynamic(this, &UMotionListWidget::OnTabGestureClicked);
	}
	if (CloseListBtn != nullptr)
	{
		CloseListBtn->OnClicked.AddDynamic(this, &UMotionListWidget::OnCloseListClicked);
	}
	if (BtnApplyList != nullptr)
	{
		BtnApplyList->OnClicked.AddDynamic(this, &UMotionListWidget::OnApplyMotionClicked);
	}
	if (SearchBox != nullptr)
	{
		SearchBox->OnTextChanged.AddDynamic(this, &UMotionListWidget::OnSearchTextChanged);
	}
	if (StarFilterBtn != nullptr)
	{
		StarFilterBtn->OnClicked.AddDynamic(this, &UMotionListWidget::OnStarFilterBtnClicked);
	}
}

void UMotionListWidget::NativeDestruct()
{
	if (TabCostumeBtn != nullptr)
	{
		TabCostumeBtn->OnClicked.RemoveDynamic(this, &UMotionListWidget::OnTabCostumeClicked);
	}
	if (TabGestureBtn != nullptr)
	{
		TabGestureBtn->OnClicked.RemoveDynamic(this, &UMotionListWidget::OnTabGestureClicked);
	}
	if (CloseListBtn != nullptr)
	{
		CloseListBtn->OnClicked.RemoveDynamic(this, &UMotionListWidget::OnCloseListClicked);
	}
	if (BtnApplyList != nullptr)
	{
		BtnApplyList->OnClicked.RemoveDynamic(this, &UMotionListWidget::OnApplyMotionClicked);
	}
	if (SearchBox != nullptr)
	{
		SearchBox->OnTextChanged.RemoveDynamic(this, &UMotionListWidget::OnSearchTextChanged);
	}
	if (StarFilterBtn != nullptr)
	{
		StarFilterBtn->OnClicked.RemoveDynamic(this, &UMotionListWidget::OnStarFilterBtnClicked);
	}

	Super::NativeDestruct();
}

// ============================================================
// InitWidget
// ============================================================
void UMotionListWidget::InitWidget(UCustomizingMotionComponent* InComp)
{
	MotionComp         = InComp;
	TargetSlotIndex    = -1;
	PendingMotionIndex = -1;
	ActiveTab          = 0;
	CurrentSearchText.Empty();
	bShowBookmarksOnly = false;

	if (SearchBox != nullptr)
	{
		SearchBox->SetText(FText::GetEmpty());
	}

	RefreshMotionList();
}

// ============================================================
// SetTargetSlot
// ============================================================
void UMotionListWidget::SetTargetSlot(int32 InSlotIndex)
{
	TargetSlotIndex    = InSlotIndex;
	PendingMotionIndex = -1;
	RefreshMotionList();
}

// ============================================================
// RefreshMotionList
// ============================================================
void UMotionListWidget::RefreshMotionList()
{
	if (MotionComp == nullptr || MotionBodyScroll == nullptr)
	{
		return;
	}
	MotionBodyScroll->ClearChildren();
	DisplayedIndices.Empty();

	const TArray<FMotionSlotData>& Av  = MotionComp->GetAvailableMotions();
	const TArray<FMotionSlotData>& BMs = MotionComp->GetBookmarks();

	if (Av.Num() == 0)
	{
		UTextBlock* E = NewObject<UTextBlock>(WidgetTree);
		E->SetText(FText::FromString(TEXT("  사용 가능한 모션이 없습니다.")));
		E->SetColorAndOpacity(FSlateColor(ML::TxtDim));
		MotionBodyScroll->AddChild(E);
		return;
	}

	// ── 1. 탭 + 검색 필터 ──────────────────────────────────
	const ECustomMotionType FilterType =
		(ActiveTab == 0) ? ECustomMotionType::Item : ECustomMotionType::Social;

	TArray<int32> BookmarkedIdx;
	TArray<int32> NormalIdx;

	for (int32 i = 0; i < Av.Num(); ++i)
	{
		// 탭 필터
		if (Av[i].MotionType != ECustomMotionType::None
			&& Av[i].MotionType != FilterType)
		{
			continue;
		}

		// 검색 필터
		if (!CurrentSearchText.IsEmpty())
		{
			const FString DisplayName = Av[i].DisplayName.IsEmpty()
				? Av[i].ActionName
				: Av[i].DisplayName.ToString();

			if (!DisplayName.Contains(CurrentSearchText, ESearchCase::IgnoreCase))
			{
				continue;
			}
		}

		const bool bBookmarked = BMs.ContainsByPredicate(
			[&](const FMotionSlotData& B) { return B == Av[i]; });

		if (bShowBookmarksOnly && !bBookmarked)
		{
			continue;
		}

		if (bBookmarked)
		{
			BookmarkedIdx.Add(i);
		}
		else
		{
			NormalIdx.Add(i);
		}
	}

	// ── 2. 즐겨찾기 먼저, 그 다음 일반 항목 ────────────────
	DisplayedIndices.Append(BookmarkedIdx);
	DisplayedIndices.Append(NormalIdx);

	if (DisplayedIndices.Num() == 0)
	{
		UTextBlock* None = NewObject<UTextBlock>(WidgetTree);
		None->SetText(FText::FromString(TEXT("  해당 탭에 모션이 없습니다.")));
		None->SetColorAndOpacity(FSlateColor(ML::TxtDim));
		MotionBodyScroll->AddChild(None);
		return;
	}

	// ── 3. 최대 10행 렌더링 ──────────────────────────────────
	const int32 MaxRows = FMath::Min(DisplayedIndices.Num(), 10);

	for (int32 EntryIdx = 0; EntryIdx < MaxRows; ++EntryIdx)
	{
		const int32 AvIdx    = DisplayedIndices[EntryIdx];
		const bool  bSel     = (AvIdx == PendingMotionIndex);
		const bool  bBookmark = BMs.ContainsByPredicate(
			[&](const FMotionSlotData& B) { return B == Av[AvIdx]; });

		UBorder* RowBg = NewObject<UBorder>(WidgetTree);
		RowBg->SetBrushColor(bSel ? ML::BgRowSel : ML::BgRow);
		RowBg->SetPadding(FMargin(0.f, 0.f, 0.f, 1.f));

		UHorizontalBox* RowHB = NewObject<UHorizontalBox>(WidgetTree);
		RowBg->AddChild(RowHB);
		MotionBodyScroll->AddChild(RowBg);

		const FString MName = Av[AvIdx].DisplayName.IsEmpty()
			? Av[AvIdx].ActionName
			: Av[AvIdx].DisplayName.ToString();

		// ── 모션 이름 버튼 ────────────────────────────────
		UButton* MBtn = NewObject<UButton>(WidgetTree);
		MBtn->SetBackgroundColor(bSel ? ML::BgRowSel : ML::BgRow);

		FButtonStyle MS = MBtn->GetStyle();
		MS.NormalPadding  = FMargin(12.f, 9.f);
		MS.PressedPadding = MS.NormalPadding;
		MBtn->SetStyle(MS);

		UTextBlock* MTxt = NewObject<UTextBlock>(WidgetTree);
		MTxt->SetText(FText::FromString(MName));
		MTxt->SetColorAndOpacity(FSlateColor(bSel ? ML::TxtSelHl : ML::TxtMain));

		FSlateFontInfo MF = MTxt->GetFont();
		MF.Size = 13;
		MTxt->SetFont(MF);

		MBtn->AddChild(MTxt);

		FScriptDelegate LD;
		LD.BindUFunction(this, FName(*FString::Printf(TEXT("L%d"), EntryIdx)));
		MBtn->OnClicked.Add(LD);

		auto* MBS = RowHB->AddChildToHorizontalBox(MBtn);
		MBS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		// ── ★ 북마크 버튼 ────────────────────────────────
		UButton* StarBtn = NewObject<UButton>(WidgetTree);
		StarBtn->SetBackgroundColor(bBookmark ? ML::BgStarOn : ML::BgStar);

		FButtonStyle SS = StarBtn->GetStyle();
		SS.NormalPadding  = FMargin(6.f, 9.f);
		SS.PressedPadding = SS.NormalPadding;
		StarBtn->SetStyle(SS);

		UTextBlock* StarTxt = NewObject<UTextBlock>(WidgetTree);
		StarTxt->SetText(FText::FromString(TEXT("★")));
		StarTxt->SetColorAndOpacity(FSlateColor(bBookmark ? ML::TxtStar : ML::TxtDim));

		FSlateFontInfo SF = StarTxt->GetFont();
		SF.Size = 14;
		StarTxt->SetFont(SF);

		StarBtn->AddChild(StarTxt);

		FScriptDelegate BD;
		BD.BindUFunction(this, FName(*FString::Printf(TEXT("B%d"), EntryIdx)));
		StarBtn->OnClicked.Add(BD);

		USizeBox* StarSz = NewObject<USizeBox>(WidgetTree);
		StarSz->SetWidthOverride(36.f);
		StarSz->AddChild(StarBtn);
		RowHB->AddChildToHorizontalBox(StarSz);
	}
}

// ============================================================
// SelectListMotion / L0~L9  (EntryIdx → DisplayedIndices → AvIdx)
// ============================================================
void UMotionListWidget::SelectListMotion(int32 EntryIdx)
{
	if (MotionComp == nullptr || !DisplayedIndices.IsValidIndex(EntryIdx))
	{
		return;
	}
	const int32 AvIdx = DisplayedIndices[EntryIdx];
	PendingMotionIndex = (PendingMotionIndex == AvIdx) ? -1 : AvIdx;
	RefreshMotionList();
}

void UMotionListWidget::L0()
{
	SelectListMotion(0);
}

void UMotionListWidget::L1()
{
	SelectListMotion(1);
}

void UMotionListWidget::L2()
{
	SelectListMotion(2);
}

void UMotionListWidget::L3()
{
	SelectListMotion(3);
}

void UMotionListWidget::L4()
{
	SelectListMotion(4);
}

void UMotionListWidget::L5()
{
	SelectListMotion(5);
}

void UMotionListWidget::L6()
{
	SelectListMotion(6);
}

void UMotionListWidget::L7()
{
	SelectListMotion(7);
}

void UMotionListWidget::L8()
{
	SelectListMotion(8);
}

void UMotionListWidget::L9()
{
	SelectListMotion(9);
}

// ============================================================
// ToggleBookmark / B0~B9  (EntryIdx → DisplayedIndices → AvIdx)
// ============================================================
void UMotionListWidget::ToggleBookmark(int32 EntryIdx)
{
	if (MotionComp == nullptr || !DisplayedIndices.IsValidIndex(EntryIdx))
	{
		return;
	}
	const int32 AvIdx = DisplayedIndices[EntryIdx];
	const TArray<FMotionSlotData>& Av  = MotionComp->GetAvailableMotions();
	const TArray<FMotionSlotData>& BMs = MotionComp->GetBookmarks();

	int32 BIdx = BMs.IndexOfByPredicate(
		[&](const FMotionSlotData& B) { return B == Av[AvIdx]; });

	if (BIdx >= 0)
	{
		MotionComp->RemoveBookmark(BIdx);
	}
	else
	{
		MotionComp->AddBookmark(Av[AvIdx]);
	}

	RefreshMotionList();
}

void UMotionListWidget::B0()
{
	ToggleBookmark(0);
}

void UMotionListWidget::B1()
{
	ToggleBookmark(1);
}

void UMotionListWidget::B2()
{
	ToggleBookmark(2);
}

void UMotionListWidget::B3()
{
	ToggleBookmark(3);
}

void UMotionListWidget::B4()
{
	ToggleBookmark(4);
}

void UMotionListWidget::B5()
{
	ToggleBookmark(5);
}

void UMotionListWidget::B6()
{
	ToggleBookmark(6);
}

void UMotionListWidget::B7()
{
	ToggleBookmark(7);
}

void UMotionListWidget::B8()
{
	ToggleBookmark(8);
}

void UMotionListWidget::B9()
{
	ToggleBookmark(9);
}

// ============================================================
// 탭 / 닫기 / 적용 핸들러
// ============================================================
void UMotionListWidget::OnTabCostumeClicked()
{
	ActiveTab = 0;

	if (TabCostumeBtn != nullptr)
	{
		TabCostumeBtn->SetBackgroundColor(ML::BgTabOn);
	}
	if (TabGestureBtn != nullptr)
	{
		TabGestureBtn->SetBackgroundColor(ML::BgTabOff);
	}
	if (TabCostumeTxt != nullptr)
	{
		TabCostumeTxt->SetColorAndOpacity(FSlateColor(ML::TxtTitle));
	}
	if (TabGestureTxt != nullptr)
	{
		TabGestureTxt->SetColorAndOpacity(FSlateColor(ML::TxtDim));
	}

	RefreshMotionList();
}

void UMotionListWidget::OnTabGestureClicked()
{
	ActiveTab = 1;

	if (TabCostumeBtn != nullptr)
	{
		TabCostumeBtn->SetBackgroundColor(ML::BgTabOff);
	}
	if (TabGestureBtn != nullptr)
	{
		TabGestureBtn->SetBackgroundColor(ML::BgTabOn);
	}
	if (TabCostumeTxt != nullptr)
	{
		TabCostumeTxt->SetColorAndOpacity(FSlateColor(ML::TxtDim));
	}
	if (TabGestureTxt != nullptr)
	{
		TabGestureTxt->SetColorAndOpacity(FSlateColor(ML::TxtTitle));
	}

	RefreshMotionList();
}

void UMotionListWidget::OnCloseListClicked()
{
	TargetSlotIndex    = -1;
	PendingMotionIndex = -1;
	OnCloseRequested.Broadcast();
}

void UMotionListWidget::OnApplyMotionClicked()
{
	if (MotionComp == nullptr || TargetSlotIndex < 0 || PendingMotionIndex < 0)
	{
		return;
	}
	const TArray<FMotionSlotData>& Av = MotionComp->GetAvailableMotions();
	if (Av.IsValidIndex(PendingMotionIndex) == false)
	{
		return;
	}

	MotionComp->SetSlot(TargetSlotIndex, Av[PendingMotionIndex]);
	OnMotionApplied.Broadcast(TargetSlotIndex, Av[PendingMotionIndex]);

	PendingMotionIndex = -1;
	RefreshMotionList();
}

// ============================================================
// 검색 / 즐겨찾기 필터 핸들러
// ============================================================
void UMotionListWidget::OnSearchTextChanged(const FText& Text)
{
	CurrentSearchText = Text.ToString();
	PendingMotionIndex = -1;
	RefreshMotionList();
}

void UMotionListWidget::OnStarFilterBtnClicked()
{
	bShowBookmarksOnly = !bShowBookmarksOnly;

	if (StarFilterBtn != nullptr)
	{
		StarFilterBtn->SetBackgroundColor(
			bShowBookmarksOnly ? ML::BgStarOn : ML::BgStar);
	}
	if (StarFilterTxt != nullptr)
	{
		StarFilterTxt->SetColorAndOpacity(
			FSlateColor(bShowBookmarksOnly ? ML::TxtStar : ML::TxtDim));
	}

	PendingMotionIndex = -1;
	RefreshMotionList();
}
