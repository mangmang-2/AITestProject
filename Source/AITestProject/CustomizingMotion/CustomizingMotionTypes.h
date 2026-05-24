// CustomizingMotionTypes.h
// Customizing Motion 시스템 공용 타입 정의

#pragma once

#include "CoreMinimal.h"
#include "CustomizingMotionTypes.generated.h"

class UAnimMontage;
class UAnimSequenceBase;

UENUM(BlueprintType)
enum class ECustomMotionType : uint8
{
	None   UMETA(DisplayName = "None"),
	Item   UMETA(DisplayName = "Item Motion"),
	Social UMETA(DisplayName = "Social Motion"),
};

UENUM(BlueprintType)
enum class ECostumeGrade : uint8
{
	None      UMETA(DisplayName = "None"),
	Edge      UMETA(DisplayName = "Edge (3 slots)"),
	Prestige  UMETA(DisplayName = "Prestige (5 slots)"),
};

USTRUCT(BlueprintType)
struct FMotionSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	int32 Index = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	ECustomMotionType MotionType = ECustomMotionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	int32 ClassID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	FString ActionName = TEXT("n");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	TSoftObjectPtr<UAnimSequenceBase> AnimSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	int32 BeginFrame = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	int32 EndFrame = 0;

	bool IsValid() const
	{
		return MotionType != ECustomMotionType::None
			&& (!Montage.IsNull() || !AnimSequence.IsNull());
	}

	bool operator==(const FMotionSlotData& Other) const
	{
		return MotionType == Other.MotionType
			&& ClassID == Other.ClassID
			&& ActionName == Other.ActionName;
	}
};

USTRUCT(BlueprintType)
struct FMotionPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	FString PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	TArray<FMotionSlotData> Slots;
};

namespace CustomMotionConstants
{
	inline constexpr int32 MaxSlots_Prestige = 5;
	inline constexpr int32 MaxSlots_Edge     = 3;
	inline constexpr int32 MaxPresets        = 10;
	inline constexpr int32 MaxBookmarks      = 200;

	inline int32 GetMaxSlots(ECostumeGrade Grade)
	{
		switch (Grade)
		{
		case ECostumeGrade::Prestige: return MaxSlots_Prestige;
		case ECostumeGrade::Edge:     return MaxSlots_Edge;
		default:                      return 0;
		}
	}
}
