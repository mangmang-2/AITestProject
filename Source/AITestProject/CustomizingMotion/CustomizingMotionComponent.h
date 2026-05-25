// CustomizingMotionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomizingMotionTypes.h"
#include "CustomizingMotionComponent.generated.h"

class UAnimInstance;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMotionSlotsChanged, const TArray<FMotionSlotData>&, Slots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMotionPlayStateChanged, bool, bIsPlaying);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AITESTPROJECT_API UCustomizingMotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCustomizingMotionComponent();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	bool SetSlot(int32 SlotIndex, const FMotionSlotData& Data);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void ClearSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void ClearAllSlots();

	/** 두 슬롯을 교체 (OnMotionSlotsChanged 1회만 발생) */
	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void SwapSlots(int32 IndexA, int32 IndexB);

	/** From 슬롯을 To 위치로 이동 — 사이 슬롯들은 한 칸씩 밀림 */
	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void MoveSlot(int32 From, int32 To);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	const TArray<FMotionSlotData>& GetSlots() const { return ActiveSlots; }

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion")
	int32 GetValidSlotCount() const;

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void SetCostumeGrade(ECostumeGrade NewGrade);

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion")
	ECostumeGrade GetCostumeGrade() const { return CurrentGrade; }

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion")
	int32 GetMaxSlotCount() const;

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void StartMotionLoop();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void StopMotionLoop();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void PauseMotion();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void ResumeMotion();

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void PlaySingleSlot(int32 SlotIndex, bool bLoop = false);

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion")
	bool IsPlaying() const { return bIsPlaying; }

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion")
	bool CanPlayMotion(FString& OutReason) const;

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion|Preset")
	bool SavePreset(int32 PresetIndex, const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion|Preset")
	bool LoadPreset(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion|Preset")
	bool DeletePreset(int32 PresetIndex);

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion|Preset")
	const TArray<FMotionPreset>& GetPresets() const { return Presets; }

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion|Bookmark")
	bool AddBookmark(const FMotionSlotData& Data);

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion|Bookmark")
	bool RemoveBookmark(int32 BookmarkIndex);

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion|Bookmark")
	const TArray<FMotionSlotData>& GetBookmarks() const { return Bookmarks; }

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	const TArray<FMotionSlotData>& GetAvailableMotions() const { return AvailableMotions; }

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion")
	void SetAvailableMotions(const TArray<FMotionSlotData>& Motions);

	UFUNCTION(BlueprintPure, Category = "CustomizingMotion|Serialize")
	FString SerializeSlots() const;

	UFUNCTION(BlueprintCallable, Category = "CustomizingMotion|Serialize")
	bool DeserializeSlots(const FString& Data);

	UPROPERTY(BlueprintAssignable, Category = "CustomizingMotion")
	FOnMotionSlotsChanged OnMotionSlotsChanged;

	UPROPERTY(BlueprintAssignable, Category = "CustomizingMotion")
	FOnMotionPlayStateChanged OnMotionPlayStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void PlayNextInLoop();
	UFUNCTION() void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UAnimMontage* ResolveMontage(const FMotionSlotData& Slot);
	int32 FindNextValidIndex(int32 Current) const;
	UAnimInstance* GetOwnerAnimInstance() const;

private:
	UPROPERTY(EditAnywhere, Category = "CustomizingMotion")
	ECostumeGrade CurrentGrade = ECostumeGrade::Prestige;

	UPROPERTY(VisibleAnywhere, Category = "CustomizingMotion")
	TArray<FMotionSlotData> ActiveSlots;

	UPROPERTY(EditAnywhere, Category = "CustomizingMotion")
	TArray<FMotionSlotData> AvailableMotions;

	UPROPERTY(VisibleAnywhere, Category = "CustomizingMotion")
	TArray<FMotionPreset> Presets;

	UPROPERTY(VisibleAnywhere, Category = "CustomizingMotion")
	TArray<FMotionSlotData> Bookmarks;

	bool bIsPlaying = false;
	bool bIsPaused = false;
	int32 CurrentPlayIndex = -1;
	FDelegateHandle MontageEndedHandle;
};
