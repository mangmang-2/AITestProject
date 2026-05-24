// MotionListEntryWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMotionTypes.h"
#include "MotionListEntryWidget.generated.h"

class UCustomizingMotionComponent;
class UTextBlock;
class UButton;

UCLASS()
class AITESTPROJECT_API UMotionListEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void Init(const FMotionSlotData& InData, UCustomizingMotionComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void OnAssignToSlot(int32 TargetSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "MotionList")
	void OnPreview();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* EntryNameText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* AssignButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* PreviewButton = nullptr;

private:
	FMotionSlotData MotionData;

	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;
};
