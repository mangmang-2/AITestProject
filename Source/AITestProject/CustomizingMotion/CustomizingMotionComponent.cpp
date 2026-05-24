#include "CustomizingMotionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"

UCustomizingMotionComponent::UCustomizingMotionComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UCustomizingMotionComponent::BeginPlay()
{
	Super::BeginPlay();
	const int32 MaxSlots = GetMaxSlotCount();
	ActiveSlots.SetNum(MaxSlots);
	for (int32 i = 0; i < MaxSlots; ++i) ActiveSlots[i].Index = i;
	Presets.SetNum(CustomMotionConstants::MaxPresets);
}

void UCustomizingMotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopMotionLoop();
	Super::EndPlay(EndPlayReason);
}

bool UCustomizingMotionComponent::SetSlot(int32 SlotIndex, const FMotionSlotData& Data)
{
	if (SlotIndex < 0 || SlotIndex >= GetMaxSlotCount()) return false;
	FMotionSlotData NewData = Data; NewData.Index = SlotIndex;
	ActiveSlots[SlotIndex] = NewData;
	OnMotionSlotsChanged.Broadcast(ActiveSlots);
	return true;
}

void UCustomizingMotionComponent::ClearSlot(int32 SlotIndex)
{
	if (!ActiveSlots.IsValidIndex(SlotIndex)) return;
	ActiveSlots[SlotIndex] = FMotionSlotData(); ActiveSlots[SlotIndex].Index = SlotIndex;
	OnMotionSlotsChanged.Broadcast(ActiveSlots);
}

void UCustomizingMotionComponent::ClearAllSlots()
{
	const int32 MaxSlots = GetMaxSlotCount();
	ActiveSlots.SetNum(MaxSlots);
	for (int32 i = 0; i < MaxSlots; ++i) { ActiveSlots[i] = FMotionSlotData(); ActiveSlots[i].Index = i; }
	OnMotionSlotsChanged.Broadcast(ActiveSlots);
}

int32 UCustomizingMotionComponent::GetValidSlotCount() const
{
	int32 Count = 0;
	const int32 MaxSlots = GetMaxSlotCount();
	for (int32 i = 0; i < FMath::Min(ActiveSlots.Num(), MaxSlots); ++i)
		if (ActiveSlots[i].IsValid()) ++Count;
	return Count;
}

void UCustomizingMotionComponent::SetCostumeGrade(ECostumeGrade NewGrade)
{
	if (CurrentGrade == NewGrade) return;
	const bool bWasPlaying = bIsPlaying;
	if (bWasPlaying) StopMotionLoop();
	CurrentGrade = NewGrade;
	const int32 MaxSlots = GetMaxSlotCount();
	ActiveSlots.SetNum(MaxSlots);
	for (int32 i = 0; i < MaxSlots; ++i) ActiveSlots[i].Index = i;
	OnMotionSlotsChanged.Broadcast(ActiveSlots);
	if (bWasPlaying) StartMotionLoop();
}

int32 UCustomizingMotionComponent::GetMaxSlotCount() const { return CustomMotionConstants::GetMaxSlots(CurrentGrade); }

void UCustomizingMotionComponent::StartMotionLoop()
{
	if (GetValidSlotCount() == 0) return;
	FString Reason;
	if (!CanPlayMotion(Reason)) { UE_LOG(LogTemp, Warning, TEXT("[CustomizingMotion] Cannot play: %s"), *Reason); return; }
	bIsPlaying = true; bIsPaused = false; CurrentPlayIndex = -1;
	OnMotionPlayStateChanged.Broadcast(true);
	PlayNextInLoop();
}

void UCustomizingMotionComponent::StopMotionLoop()
{
	if (!bIsPlaying) return;
	bIsPlaying = false; bIsPaused = false; CurrentPlayIndex = -1;
	if (UAnimInstance* AnimInst = GetOwnerAnimInstance()) AnimInst->StopAllMontages(0.25f);
	OnMotionPlayStateChanged.Broadcast(false);
}

void UCustomizingMotionComponent::PauseMotion()
{
	if (!bIsPlaying || bIsPaused) return;
	bIsPaused = true;
	if (UAnimInstance* AnimInst = GetOwnerAnimInstance()) AnimInst->Montage_Pause();
}

void UCustomizingMotionComponent::ResumeMotion()
{
	if (!bIsPlaying || !bIsPaused) return;
	bIsPaused = false;
	if (UAnimInstance* AnimInst = GetOwnerAnimInstance()) AnimInst->Montage_Resume(nullptr);
}

void UCustomizingMotionComponent::PlaySingleSlot(int32 SlotIndex, bool bLoop)
{
	if (!ActiveSlots.IsValidIndex(SlotIndex) || !ActiveSlots[SlotIndex].IsValid()) return;
	if (bIsPlaying) StopMotionLoop();
	const FMotionSlotData& Slot = ActiveSlots[SlotIndex];
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInst = GetOwnerAnimInstance();
	if (!OwnerChar || !AnimInst) return;

	UAnimMontage* Montage = ResolveMontage(Slot);
	if (Montage) { OwnerChar->PlayAnimMontage(Montage, 1.0f); }
	else if (!Slot.AnimSequence.IsNull())
	{
		UAnimSequenceBase* Seq = Slot.AnimSequence.LoadSynchronous();
		if (Seq) AnimInst->PlaySlotAnimationAsDynamicMontage(Seq, FName("DefaultSlot"), 0.25f, 0.25f, 1.0f, bLoop ? 0 : 1);
	}
}

bool UCustomizingMotionComponent::CanPlayMotion(FString& OutReason) const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) { OutReason = TEXT("Owner is not a Character"); return false; }
	if (OwnerChar->GetCharacterMovement() && OwnerChar->GetCharacterMovement()->Velocity.SizeSquared() > 1.0f)
	{ OutReason = TEXT("Character is moving"); return false; }
	if (OwnerChar->GetCharacterMovement() && OwnerChar->GetCharacterMovement()->IsFalling())
	{ OutReason = TEXT("Character is in air"); return false; }
	if (GetValidSlotCount() == 0) { OutReason = TEXT("No valid motion slots"); return false; }
	return true;
}

bool UCustomizingMotionComponent::SavePreset(int32 PresetIndex, const FString& Name)
{
	if (PresetIndex < 0 || PresetIndex >= CustomMotionConstants::MaxPresets) return false;
	if (!Presets.IsValidIndex(PresetIndex)) Presets.SetNum(CustomMotionConstants::MaxPresets);
	Presets[PresetIndex].PresetName = Name; Presets[PresetIndex].Slots = ActiveSlots;
	return true;
}

bool UCustomizingMotionComponent::LoadPreset(int32 PresetIndex)
{
	if (!Presets.IsValidIndex(PresetIndex)) return false;
	if (Presets[PresetIndex].Slots.Num() == 0) return false;
	const bool bWasPlaying = bIsPlaying;
	if (bWasPlaying) StopMotionLoop();
	const int32 MaxSlots = GetMaxSlotCount();
	ActiveSlots.SetNum(MaxSlots);
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		ActiveSlots[i] = Presets[PresetIndex].Slots.IsValidIndex(i) ? Presets[PresetIndex].Slots[i] : FMotionSlotData();
		ActiveSlots[i].Index = i;
	}
	OnMotionSlotsChanged.Broadcast(ActiveSlots);
	if (bWasPlaying) StartMotionLoop();
	return true;
}

bool UCustomizingMotionComponent::DeletePreset(int32 PresetIndex)
{
	if (!Presets.IsValidIndex(PresetIndex)) return false;
	Presets[PresetIndex] = FMotionPreset(); return true;
}

bool UCustomizingMotionComponent::AddBookmark(const FMotionSlotData& Data)
{
	if (Bookmarks.Num() >= CustomMotionConstants::MaxBookmarks) return false;
	for (const FMotionSlotData& E : Bookmarks) if (E == Data) return false;
	Bookmarks.Add(Data); return true;
}

bool UCustomizingMotionComponent::RemoveBookmark(int32 BookmarkIndex)
{
	if (!Bookmarks.IsValidIndex(BookmarkIndex)) return false;
	Bookmarks.RemoveAt(BookmarkIndex); return true;
}

void UCustomizingMotionComponent::SetAvailableMotions(const TArray<FMotionSlotData>& Motions) { AvailableMotions = Motions; }

FString UCustomizingMotionComponent::SerializeSlots() const
{
	FString Result;
	for (const FMotionSlotData& S : ActiveSlots)
		Result += FString::Printf(TEXT("%d;%d;%d;%s;"), S.Index, (int32)S.MotionType, S.ClassID, *S.ActionName);
	return Result;
}

bool UCustomizingMotionComponent::DeserializeSlots(const FString& Data)
{
	if (Data.IsEmpty()) return false;
	TArray<FString> Tokens; Data.ParseIntoArray(Tokens, TEXT(";"), true);
	const int32 F = 4, MaxSlots = GetMaxSlotCount();
	ActiveSlots.SetNum(MaxSlots);
	for (int32 i = 0; i < MaxSlots; ++i) { ActiveSlots[i] = FMotionSlotData(); ActiveSlots[i].Index = i; }
	const int32 SlotCount = FMath::Min(Tokens.Num() / F, MaxSlots);
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const int32 B = i * F; FMotionSlotData& S = ActiveSlots[i];
		S.Index = FCString::Atoi(*Tokens[B]);
		S.MotionType = (ECustomMotionType)FCString::Atoi(*Tokens[B+1]);
		S.ClassID = FCString::Atoi(*Tokens[B+2]); S.ActionName = Tokens[B+3];
		for (const FMotionSlotData& A : AvailableMotions)
			if (A == S) { S.Montage = A.Montage; S.AnimSequence = A.AnimSequence; S.DisplayName = A.DisplayName; break; }
	}
	OnMotionSlotsChanged.Broadcast(ActiveSlots); return true;
}

void UCustomizingMotionComponent::PlayNextInLoop()
{
	if (!bIsPlaying) return;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInst = GetOwnerAnimInstance();
	if (!OwnerChar || !AnimInst) { StopMotionLoop(); return; }

	// 재귀 대신 반복문 — 모든 슬롯이 로드 실패해도 스택 오버플로우 없음
	const int32 MaxAttempts = FMath::Max(1, FMath::Min(ActiveSlots.Num(), GetMaxSlotCount()));
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const int32 NextIndex = FindNextValidIndex(CurrentPlayIndex);
		if (NextIndex < 0) { StopMotionLoop(); return; }
		CurrentPlayIndex = NextIndex;

		const FMotionSlotData& Slot = ActiveSlots[CurrentPlayIndex];
		UAnimMontage* PlayedMontage = nullptr;

		if (UAnimMontage* M = ResolveMontage(Slot)) { OwnerChar->PlayAnimMontage(M, 1.0f); PlayedMontage = M; }
		else if (!Slot.AnimSequence.IsNull())
		{
			if (UAnimSequenceBase* Seq = Slot.AnimSequence.LoadSynchronous())
				PlayedMontage = AnimInst->PlaySlotAnimationAsDynamicMontage(Seq, FName("DefaultSlot"), 0.25f, 0.25f, 1.0f, 1);
		}

		if (PlayedMontage)
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UCustomizingMotionComponent::OnMontageEnded);
			AnimInst->Montage_SetEndDelegate(EndDelegate, PlayedMontage);
			return;
		}
		// 이 슬롯 에셋 로드 실패 → 다음 슬롯 시도
		UE_LOG(LogTemp, Warning, TEXT("[CustomizingMotion] Slot %d: montage load failed, skipping"), CurrentPlayIndex);
	}

	// 모든 슬롯이 재생 불가 → 루프 중단
	UE_LOG(LogTemp, Warning, TEXT("[CustomizingMotion] No playable slots found, stopping loop"));
	StopMotionLoop();
}

void UCustomizingMotionComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bIsPlaying || bInterrupted) return;
	PlayNextInLoop();
}

UAnimMontage* UCustomizingMotionComponent::ResolveMontage(const FMotionSlotData& Slot)
{
	return Slot.Montage.IsNull() ? nullptr : Slot.Montage.LoadSynchronous();
}

int32 UCustomizingMotionComponent::FindNextValidIndex(int32 Current) const
{
	const int32 MaxSlots = FMath::Min(ActiveSlots.Num(), GetMaxSlotCount());
	if (MaxSlots == 0) return -1;
	int32 Next = (Current + 1) % MaxSlots; const int32 Start = Next;
	do { if (ActiveSlots.IsValidIndex(Next) && ActiveSlots[Next].IsValid()) return Next; Next = (Next + 1) % MaxSlots; } while (Next != Start);
	return -1;
}

UAnimInstance* UCustomizingMotionComponent::GetOwnerAnimInstance() const
{
	if (const ACharacter* C = Cast<ACharacter>(GetOwner())) return C->GetMesh()->GetAnimInstance();
	return nullptr;
}
