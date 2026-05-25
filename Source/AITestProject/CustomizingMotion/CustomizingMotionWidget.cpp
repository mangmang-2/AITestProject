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

	// MotionListWidget 델리게이트 해제
	if (MotionListWidget != nullptr)
	{
		MotionListWidget->OnCloseRequested.RemoveDynamic(this, &UCustomizingMotionWidget::OnListCloseRequested);
		MotionListWidget->OnMotionApplied.RemoveDynamic(this, &UCustomizingMotionWidget::OnMotionApplied);
	}

	Super::NativeDestruct();
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
		const FGeometry& Geo       = GetCachedGeometry();
		const FVector2D  AbsPos    = Geo.GetAbsolutePosition();  // 물리 픽셀
		const FVector2D  AbsSize   = Geo.GetAbsoluteSize();      // 물리 픽셀
		const FVector2D  LocalSize = Geo.GetLocalSize();         // 논리 픽셀 (DPI 독립)

		// 물리→논리 변환 비율 (DPI 스케일)
		const float Scale = (LocalSize.X > 0.f) ? (AbsSize.X / LocalSize.X) : 1.f;

		// 논리 좌표로 변환 후 메인 위젯 오른쪽 4px에 배치
		const FVector2D LogPos(AbsPos.X / Scale, AbsPos.Y / Scale);
		MotionListWidget->SetPositionInViewport(
			FVector2D(500 + 4.f, LogPos.Y),
			false);

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

		// OnMoveRequested → OnSlotMoveRequested
		SlotW->OnMoveRequested.AddDynamic(this, &UCustomizingMotionWidget::OnSlotMoveRequested);

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
	OnApplyRequested.Broadcast();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCustomizingMotionWidget::OnCloseBtnClicked()
{
	OnCloseRequested.Broadcast();
	SetVisibility(ESlateVisibility::Collapsed);
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
// MotionSlotWidget 델리게이트 수신
// ============================================================
void UCustomizingMotionWidget::OnSlotMoveRequested(int32 SlotIdx, int32 Direction)
{
	if (MotionComp == nullptr)
	{
		return;
	}

	const int32 TargetIdx = SlotIdx + Direction;
	const int32 MaxSlots  = MotionComp->GetMaxSlotCount();

	if (TargetIdx < 0 || TargetIdx >= MaxSlots)
	{
		return;
	}

	MotionComp->SwapSlots(SlotIdx, TargetIdx);

	// 선택 상태를 이동된 슬롯으로 유지
	if (SelectedSlotIndex == SlotIdx)
	{
		SelectedSlotIndex = TargetIdx;
	}
	else if (SelectedSlotIndex == TargetIdx)
	{
		SelectedSlotIndex = SlotIdx;
	}
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
