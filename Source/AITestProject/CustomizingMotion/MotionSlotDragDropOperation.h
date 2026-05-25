// MotionSlotDragDropOperation.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "MotionSlotDragDropOperation.generated.h"

// 드래그된 슬롯 인덱스를 전달하는 DragDrop 오퍼레이션
UCLASS()
class AITESTPROJECT_API UMotionSlotDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 DraggedSlotIndex = -1;
};
