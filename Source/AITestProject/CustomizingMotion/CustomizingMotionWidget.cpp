// CustomizingMotionWidget.cpp
#include "CustomizingMotionWidget.h"
#include "CustomizingMotionComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SViewport.h"
#include "Components/Border.h"
#include "Components/Spacer.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Animation/AnimSequenceBase.h"

// ============================================================
// 초기화
// ============================================================
bool UCustomizingMotionWidget::Initialize()
{
	if (Super::Initialize() == false)
	{
		return false;
	}
	SetIsFocusable(true);
	return true;
}

void UCustomizingMotionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// SlotWidgetClass 미설정 시 WBP 동적 로드
	if (SlotWidgetClass == nullptr)
	{
		SlotWidgetClass = LoadClass<UMotionSlotWidget>(
			nullptr,
			TEXT("/Game/UI/Customizing/WBP_MotionSlotWidget.WBP_MotionSlotWidget_C"));
	}

	// ListWidgetClass 미설정 시 WBP 동적 로드
	if (ListWidgetClass == nullptr)
	{
		ListWidgetClass = LoadClass<UMotionListWidget>(
			nullptr,
			TEXT("/Game/UI/Customizing/WBP_MotionListWidget.WBP_MotionListWidget_C"));
	}

	// 버튼 델리게이트 바인딩
	if (BtnSavePreset != nullptr)
	{
		BtnSavePreset->OnClicked.AddDynamic(this, &UCustomizingMotionWidget::OnSavePresetBtnClicked);
	}
	if (BtnReset != nullptr)
	{
		BtnReset->OnClicked.AddDynamic(this, &UCustomizingMotionWidget::OnResetAllClicked);
	}
	if (CircleBtn != nullptr)
	{
		CircleBtn->OnClicked.AddDynamic(this, &UCustomizingMotionWidget::OnCirclePlayClicked);
	}

	if (BtnApply != nullptr)
	{
		BtnApply->OnClicked.AddDynamic(this, &UCustomizingMotionWidget::OnApplyBtnClicked);
	}

	if (CloseBtn != nullptr)
	{
		CloseBtn->OnClicked.AddDynamic(this, &UCustomizingMotionWidget::OnCloseBtnClicked);
	}
}

void UCustomizingMotionWidget::NativeDestruct()
{
	// 컴포넌트 델리게이트 해제
	if (MotionComp != nullptr)
	{
		MotionComp->OnMotionSlotsChanged.RemoveDynamic(this, &UCustomizingMotionWidget::OnSlotsChanged);
		MotionComp->OnMotionPlayStateChanged.RemoveDynamic(this, &UCustomizingMotionWidget::OnPlayStateChanged);
	}

	// 버튼 델리게이트 해제
	if (BtnSavePreset != nullptr)
	{
		BtnSavePreset->OnClicked.RemoveDynamic(this, &UCustomizingMotionWidget::OnSavePresetBtnClicked);
	}
	if (BtnReset != nullptr)
	{
		BtnReset->OnClicked.RemoveDynamic(this, &UCustomizingMotionWidget::OnResetAllClicked);
	}
	if (CircleBtn != nullptr)
	{
		CircleBtn->OnClicked.RemoveDynamic(this, &UCustomizingMotionWidget::OnCirclePlayClicked);
	}
	if (BtnApply != nullptr)
	{
		BtnApply->OnClicked.RemoveDynamic(this, &UCustomizingMotionWidget::OnApplyBtnClicked);
	}
	if (CloseBtn != nullptr)
	{
		CloseBtn->OnClicked.RemoveDynamic(this, &UCustomizingMotionWidget::OnCloseBtnClicked);
	}

	// MotionListWidget 델리게이트 해제 + 뷰포트에서 제거
	if (MotionListWidget != nullptr)
	{
		MotionListWidget->OnCloseRequested.RemoveDynamic(this, &UCustomizingMotionWidget::OnListCloseRequested);
		MotionListWidget->OnMotionApplied.RemoveDynamic(this, &UCustomizingMotionWidget::OnMotionApplied);
		MotionListWidget->RemoveFromParent();
		MotionListWidget = nullptr;
	}

	Super::NativeDestruct();
}

// ============================================================
// 드래그 이동 — NativeTick
// ============================================================
void UCustomizingMotionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bIsDragging) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) { bIsDragging = false; return; }

	// FReply::Handled()로 소비된 이벤트는 UPlayerInput에 전달되지 않으므로
	// Slate 레이어의 실제 버튼 상태를 직접 확인
	if (!FSlateApplication::IsInitialized() ||
		!FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton))
	{
		bIsDragging = false;
		return;
	}

	float MouseX = 0.f, MouseY = 0.f;
	PC->GetMousePosition(MouseX, MouseY);

	const FVector2D CurrentPos(MouseX, MouseY);
	const FVector2D Delta    = CurrentPos - DragLastMousePos;
	DragLastMousePos         = CurrentPos;

	const float DPIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	DragOffset          += (DPIScale > 0.f) ? (Delta / DPIScale) : Delta;
	SetRenderTranslation(DragOffset);
}

// ============================================================
// 드래그 시작 감지 — NativeOnPreviewMouseButtonDown (터널 단계)
// ============================================================
FReply UCustomizingMotionWidget::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);

	// 위젯 로컬 좌표로 변환 (embedded 위젯이므로 AbsoluteToLocal 사용)
	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	if (LocalPos.Y < 0.f || LocalPos.Y > TitleBarHeight)
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);

	// CloseBtn 위에 있으면 버튼 클릭 허용
	if (CloseBtn != nullptr)
	{
		const FGeometry& BtnGeo  = CloseBtn->GetCachedGeometry();
		const FVector2D  BtnSize = BtnGeo.GetLocalSize();
		if (BtnSize.X > 0.f && BtnSize.Y > 0.f)
		{
			const FVector2D BtnLocal = BtnGeo.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
			if (BtnLocal.X >= 0.f && BtnLocal.X <= BtnSize.X &&
				BtnLocal.Y >= 0.f && BtnLocal.Y <= BtnSize.Y)
				return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
		}
	}

	bIsDragging = true;
	float MouseX = 0.f, MouseY = 0.f;
	PC->GetMousePosition(MouseX, MouseY);
	DragLastMousePos = FVector2D(MouseX, MouseY);
	return FReply::Handled();
}

void UCustomizingMotionWidget::InitWidget(UCustomizingMotionComponent* InComp)
{
	MotionComp = InComp;

	if (MotionComp == nullptr)
	{
		return;
	}

	// MotionListWidget 동적 생성
	if (ListWidgetClass != nullptr && MotionListWidget == nullptr)
	{
		APlayerController* PC = GetOwningPlayer();
		MotionListWidget = CreateWidget<UMotionListWidget>(PC, ListWidgetClass);

		if (MotionListWidget != nullptr)
		{
			MotionListWidget->AddToViewport(1);
			MotionListWidget->OnCloseRequested.AddDynamic(this, &UCustomizingMotionWidget::OnListCloseRequested);
			MotionListWidget->OnMotionApplied.AddDynamic(this, &UCustomizingMotionWidget::OnMotionApplied);
			MotionListWidget->InitWidget(MotionComp);
		}
	}

	PopulateTestMotions();
	MotionComp->OnMotionSlotsChanged.AddDynamic(this, &UCustomizingMotionWidget::OnSlotsChanged);
	MotionComp->OnMotionPlayStateChanged.AddDynamic(this, &UCustomizingMotionWidget::OnPlayStateChanged);

	SelectedSlotIndex = -1;

	ShowListWindow(false);
	RefreshPresetList();
	RefreshSlots();
}

// ============================================================
// ShowListWindow
// ============================================================
void UCustomizingMotionWidget::ShowListWindow(bool bShow)
{
	if (MotionListWidget == nullptr)
	{
		return;
	}

	if (bShow)
	{
		// ── 게임 뷰포트의 화면상 시작점 ────────────────────────────────
		// 에디터 PIE에서는 뷰포트가 모니터 전체가 아니므로 오프셋 보정 필요
		// 전체화면에서는 (0,0)이므로 항상 적용해도 무방
		FVector2D ViewportOffset = FVector2D::ZeroVector;
		if (GEngine && GEngine->GameViewport)
		{
			TSharedPtr<SViewport> SViewportWidget = GEngine->GameViewport->GetGameViewportWidget();
			if (SViewportWidget.IsValid())
			{
				ViewportOffset = SViewportWidget->GetCachedGeometry().GetAbsolutePosition();
			}
		}

		// ── 메인 패널의 실제 논리 좌표 계산 ────────────────────────────
		// CanvasRoot는 풀스크린이므로 실제 패널 컨테이너(VBox_Root)의 지오메트리 사용
		const FGeometry& PanelGeo  = (VBox_Root != nullptr)
		                             ? VBox_Root->GetCachedGeometry()
		                             : GetCachedGeometry();
		const FVector2D  AbsPos    = PanelGeo.GetAbsolutePosition();  // 물리 픽셀 (모니터 기준)
		const FVector2D  AbsSize   = PanelGeo.GetAbsoluteSize();      // 물리 픽셀
		const float      DPIScale  = UWidgetLayoutLibrary::GetViewportScale(this);

		// 뷰포트 기준 논리 픽셀 좌표
		const FVector2D RelAbs    = AbsPos - ViewportOffset;
		const FVector2D LogPos    = (DPIScale > 0.f) ? (RelAbs / DPIScale) : RelAbs;
		const FVector2D LogSize   = (DPIScale > 0.f) ? (AbsSize / DPIScale) : AbsSize;

		// SetRenderTranslation은 Slate 레이아웃 패스에서 CachedGeometry에 반영되므로
		// DragOffset을 별도로 더하면 2배 이동됨 → LogPos 그대로 사용
		const FVector2D ActualPos = LogPos;

		// 메인 위젯 오른쪽 4px에 배치
		MotionListWidget->SetWindowPosition(FVector2D(ActualPos.X + LogSize.X + 4.f, ActualPos.Y));
		MotionListWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MotionListWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ============================================================
// RefreshPresetList
// ============================================================
void UCustomizingMotionWidget::RefreshPresetList()
{
	if (MotionComp == nullptr || PresetListScroll == nullptr)
	{
		return;
	}
	PresetListScroll->ClearChildren();

	const TArray<FMotionPreset>& Presets = MotionComp->GetPresets();
	const int32 Max = FMath::Min(Presets.Num(), 10);

	for (int32 i = 0; i < Max; ++i)
	{
		const FString Name = Presets[i].PresetName.IsEmpty()
			? FString::Printf(TEXT("프리셋%d"), i + 1)
			: Presets[i].PresetName;
		const bool bHas = Presets[i].Slots.Num() > 0;

		UButton* PBtn = NewObject<UButton>(WidgetTree);
		PBtn->SetBackgroundColor(PresetBtnBgColor);

		FButtonStyle PS = PBtn->GetStyle();
		PS.NormalPadding  = FMargin(6.f, 4.f);
		PS.PressedPadding = PS.NormalPadding;
		PBtn->SetStyle(PS);

		UTextBlock* PTxt = NewObject<UTextBlock>(WidgetTree);
		PTxt->SetText(FText::FromString(Name));
		PTxt->SetColorAndOpacity(FSlateColor(bHas ? PresetTxtColor : PresetTxtEmptyColor));

		FSlateFontInfo PF = PTxt->GetFont();
		PF.Size = 10;
		PTxt->SetFont(PF);

		PBtn->AddChild(PTxt);

		FScriptDelegate PD;
		PD.BindUFunction(this, FName(*FString::Printf(TEXT("P%d"), i)));
		PBtn->OnClicked.Add(PD);

		PresetListScroll->AddChild(PBtn);

		// 1px 행 구분선
		UBorder* Gap = NewObject<UBorder>(WidgetTree);
		Gap->SetBrushColor(PresetDividerColor);
		Gap->SetPadding(FMargin(0.f, 1.f));
		PresetListScroll->AddChild(Gap);
	}
}

// ============================================================
// RefreshSlots — WBP_MotionSlotWidget 인스턴스로 슬롯 행 구성
// ============================================================
void UCustomizingMotionWidget::RefreshSlots()
{
	if (MotionComp == nullptr || SlotRowsBox == nullptr)
	{
		return;
	}

	SlotRowsBox->ClearChildren();
	SlotWidgets.Reset();

	if (SlotWidgetClass == nullptr)
	{
		return;
	}

	APlayerController* PC      = GetOwningPlayer();
	const int32        MaxSlots = MotionComp->GetMaxSlotCount();

	static const FName SlotFuncNames[] = {
		FName(TEXT("S0")), FName(TEXT("S1")), FName(TEXT("S2")),
		FName(TEXT("S3")), FName(TEXT("S4"))
	};

	for (int32 i = 0; i < MaxSlots && i < 5; ++i)
	{
		UMotionSlotWidget* SlotW = CreateWidget<UMotionSlotWidget>(PC, SlotWidgetClass);
		if (SlotW == nullptr)
		{
			continue;
		}

		SlotW->InitSlot(i, MotionComp);
		SlotW->SetSelected(i == SelectedSlotIndex);

		// OnSlotClicked → Si (S0~S4)
		FScriptDelegate SD;
		SD.BindUFunction(this, SlotFuncNames[i]);
		SlotW->OnSlotClicked.Add(SD);

		// OnSlotDropped → OnSlotReordered
		SlotW->OnSlotDropped.AddDynamic(this, &UCustomizingMotionWidget::OnSlotReordered);

		SlotWidgets.Add(SlotW);
		SlotRowsBox->AddChildToVerticalBox(SlotW);
	}
}

// ============================================================
// SelectSlot
// ============================================================
void UCustomizingMotionWidget::SelectSlot(int32 Index)
{
	if (SelectedSlotIndex == Index)
	{
		SelectedSlotIndex = -1;
		ShowListWindow(false);
	}
	else
	{
		SelectedSlotIndex = Index;

		if (MotionListWidget != nullptr)
		{
			MotionListWidget->SetTargetSlot(Index);
		}
		ShowListWindow(true);
	}
	RefreshSlots();
}

void UCustomizingMotionWidget::S0()
{
	SelectSlot(0);
}

void UCustomizingMotionWidget::S1()
{
	SelectSlot(1);
}

void UCustomizingMotionWidget::S2()
{
	SelectSlot(2);
}

void UCustomizingMotionWidget::S3()
{
	SelectSlot(3);
}

void UCustomizingMotionWidget::S4()
{
	SelectSlot(4);
}

// ============================================================
// 프리셋 로드 (P0~P9)
// ============================================================
void UCustomizingMotionWidget::LoadPresetAndRefresh(int32 Idx)
{
	if (MotionComp != nullptr)
	{
		MotionComp->LoadPreset(Idx);
	}
	RefreshSlots();
}

void UCustomizingMotionWidget::P0()
{
	LoadPresetAndRefresh(0);
}

void UCustomizingMotionWidget::P1()
{
	LoadPresetAndRefresh(1);
}

void UCustomizingMotionWidget::P2()
{
	LoadPresetAndRefresh(2);
}

void UCustomizingMotionWidget::P3()
{
	LoadPresetAndRefresh(3);
}

void UCustomizingMotionWidget::P4()
{
	LoadPresetAndRefresh(4);
}

void UCustomizingMotionWidget::P5()
{
	LoadPresetAndRefresh(5);
}

void UCustomizingMotionWidget::P6()
{
	LoadPresetAndRefresh(6);
}

void UCustomizingMotionWidget::P7()
{
	LoadPresetAndRefresh(7);
}

void UCustomizingMotionWidget::P8()
{
	LoadPresetAndRefresh(8);
}

void UCustomizingMotionWidget::P9()
{
	LoadPresetAndRefresh(9);
}

// ============================================================
// 버튼 핸들러
// ============================================================
void UCustomizingMotionWidget::OnResetAllClicked()
{
	if (MotionComp != nullptr)
	{
		MotionComp->ClearAllSlots();
	}
	RefreshSlots();
}

void UCustomizingMotionWidget::OnCirclePlayClicked()
{
	if (MotionComp == nullptr)
	{
		return;
	}
	if (MotionComp->IsPlaying())
	{
		MotionComp->StopMotionLoop();
	}
	else
	{
		MotionComp->StartMotionLoop();
	}
}

void UCustomizingMotionWidget::OnApplyBtnClicked()
{
	SelectedSlotIndex = -1;
	ShowListWindow(false);
	OnApplyRequested.Broadcast();
	RemoveFromParent(); // NativeDestruct에서 MotionListWidget 정리, ToggleMotionUI에서 InitWidget 재호출
}

void UCustomizingMotionWidget::OnCloseBtnClicked()
{
	SelectedSlotIndex = -1;
	ShowListWindow(false);
	OnCloseRequested.Broadcast();
	RemoveFromParent();
}

void UCustomizingMotionWidget::OnSavePresetBtnClicked()
{
	if (MotionComp == nullptr)
	{
		return;
	}
	MotionComp->SavePreset(0, TEXT("프리셋1"));
	RefreshPresetList();
}

// ============================================================
// MotionSlotWidget 드래그 드롭 수신
// ============================================================
void UCustomizingMotionWidget::OnSlotReordered(int32 FromSlot, int32 ToSlot)
{
	if (MotionComp == nullptr)
	{
		return;
	}

	MotionComp->MoveSlot(FromSlot, ToSlot);

	// 선택된 슬롯 인덱스 보정
	if (SelectedSlotIndex == FromSlot)
	{
		SelectedSlotIndex = ToSlot;
	}
	else if (FromSlot < ToSlot && SelectedSlotIndex > FromSlot && SelectedSlotIndex <= ToSlot)
	{
		SelectedSlotIndex -= 1;
	}
	else if (FromSlot > ToSlot && SelectedSlotIndex >= ToSlot && SelectedSlotIndex < FromSlot)
	{
		SelectedSlotIndex += 1;
	}
	// RefreshSlots 는 OnMotionSlotsChanged → OnSlotsChanged 경로로 자동 호출됨
}

// ============================================================
// MotionListWidget 델리게이트 수신
// ============================================================
void UCustomizingMotionWidget::OnListCloseRequested()
{
	SelectedSlotIndex = -1;
	ShowListWindow(false);
	RefreshSlots();
}

void UCustomizingMotionWidget::OnMotionApplied(int32 SlotIndex, FMotionSlotData MotionData)
{
	RefreshSlots();
}

// ============================================================
// 컴포넌트 델리게이트 콜백
// ============================================================
void UCustomizingMotionWidget::OnSlotsChanged(const TArray<FMotionSlotData>& Slots)
{
	RefreshSlots();
}

void UCustomizingMotionWidget::OnPlayStateChanged(bool bPlaying)
{
	if (CircleStateTxt != nullptr)
	{
		CircleStateTxt->SetColorAndOpacity(
			FSlateColor(bPlaying ? PlayingIndicatorColor : StoppedIndicatorColor));
	}
}

// ============================================================
// 기존 BP 호환 함수
// ============================================================
void UCustomizingMotionWidget::OnSavePresetClicked(int32 PresetIndex)
{
	if (MotionComp != nullptr)
	{
		MotionComp->SavePreset(PresetIndex,
			FString::Printf(TEXT("프리셋%d"), PresetIndex + 1));
	}
	RefreshPresetList();
}

void UCustomizingMotionWidget::OnLoadPresetClicked(int32 PresetIndex)
{
	if (MotionComp != nullptr)
	{
		MotionComp->LoadPreset(PresetIndex);
	}
	RefreshSlots();
}

void UCustomizingMotionWidget::OnDeletePresetClicked(int32 PresetIndex)
{
	if (MotionComp != nullptr)
	{
		MotionComp->DeletePreset(PresetIndex);
	}
	RefreshPresetList();
}

void UCustomizingMotionWidget::OnPlayClicked()
{
	if (MotionComp != nullptr)
	{
		MotionComp->StartMotionLoop();
	}
}

void UCustomizingMotionWidget::OnStopClicked()
{
	if (MotionComp != nullptr)
	{
		MotionComp->StopMotionLoop();
	}
}

void UCustomizingMotionWidget::AssignMotion(int32 MotionIndex)
{
	if (MotionComp == nullptr || SelectedSlotIndex < 0)
	{
		return;
	}
	const TArray<FMotionSlotData>& Av = MotionComp->GetAvailableMotions();
	if (Av.IsValidIndex(MotionIndex))
	{
		MotionComp->SetSlot(SelectedSlotIndex, Av[MotionIndex]);
	}
}

void UCustomizingMotionWidget::OnClearSlotClicked()
{
	if (MotionComp != nullptr && SelectedSlotIndex >= 0)
	{
		MotionComp->ClearSlot(SelectedSlotIndex);
	}
}

void UCustomizingMotionWidget::OnSaveClicked()
{
	if (MotionComp == nullptr)
	{
		return;
	}
	const FString S = MotionComp->SerializeSlots();
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
			FString::Printf(TEXT("[Save] %s"), *S));
	}
}

// ============================================================
// PopulateTestMotions
// ============================================================
void UCustomizingMotionWidget::PopulateTestMotions()
{
	if (MotionComp == nullptr || MotionComp->GetAvailableMotions().Num() > 0)
	{
		return;
	}

	TArray<FMotionSlotData> T;
	auto Add = [&](const FString& Name, const FString& Path, ECustomMotionType Type, int32 CID = 0)
	{
		FMotionSlotData D;
		D.MotionType   = Type;
		D.ActionName   = Name;
		D.DisplayName  = FText::FromString(Name);
		D.ClassID      = CID;
		D.AnimSequence = TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(Path));
		T.Add(D);
	};

	const FString BU  = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/");
	const FString BUW = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Walk/");
	const FString BUJ = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/");

	Add(TEXT("대기"),      BU  + TEXT("MM_Idle.MM_Idle"),                         ECustomMotionType::Social);
	Add(TEXT("걷기"),      BUW + TEXT("MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd"), ECustomMotionType::Social);
	Add(TEXT("뒤로 걷기"), BUW + TEXT("MF_Unarmed_Walk_Bwd.MF_Unarmed_Walk_Bwd"), ECustomMotionType::Social);
	Add(TEXT("점프"),      BUJ + TEXT("MM_Jump.MM_Jump"),                         ECustomMotionType::Social);
	Add(TEXT("낙하"),      BUJ + TEXT("MM_Fall_Loop.MM_Fall_Loop"),               ECustomMotionType::Social);
	Add(TEXT("착지"),      BUJ + TEXT("MM_Land.MM_Land"),                         ECustomMotionType::Item, 1001);

	MotionComp->SetAvailableMotions(T);
}
