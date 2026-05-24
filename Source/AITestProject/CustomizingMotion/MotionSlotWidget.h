// MotionSlotWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMotionTypes.h"
#include "MotionSlotWidget.generated.h"

class UCustomizingMotionComponent;
class UTextBlock;
class UButton;

UCLASS()
class AITESTPROJECT_API UMotionSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void InitSlot(int32 InSlotIndex, UCustomizingMotionComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = "MotionSlot")
	void OnClearClicked();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SlotLabel = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MotionNameText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ClearButton = nullptr;

private:
	int32 SlotIndex = -1;

	UPROPERTY()
	UCustomizingMotionComponent* MotionComp = nullptr;
};
