# Customizing Motion UI — Step 2 프롬프트

## 완료된 작업 (Step 1)
- `CustomizingMotionWidget.h/.cpp` 완전 재작성
- 이미지 기준 **메인창(430×290)** + **리스트창(265×500)** 두 패널 분리
- 메인창: 타이틀바 / 프리셋 목록(왼쪽) / 슬롯 5개(오른쪽) / 하단버튼(프리셋 저장·초기화·적용·●)
- 리스트창: 타이틀바 / 검색바(플레이스홀더) / 탭(의상 모션·제스처) / 모션 목록+★ / 하단[적용]
- 슬롯 클릭 → 리스트창 열림, 모션 선택 → [적용] 클릭 → 슬롯에 할당
- ● 버튼 → 모션 루프 재생/정지, 색 변화(재생=초록)
- 탭 필터링: Item 타입 = 의상 모션, Social 타입 = 제스처
- ★ 버튼 → 북마크 토글

## 현재 한계 / Step 2 에서 개선할 것

### 2-1. 실제 검색 기능 구현
- 현재: TextBlock 플레이스홀더만 존재
- 목표: `UEditableTextBox` 로 교체, 입력 시 `RefreshMotionList()` 실시간 필터링
- 관련 파일: `CustomizingMotionWidget.h` (SearchBox 멤버 추가), `.cpp` (BuildLayoutFromCode 검색바 교체, RefreshMotionList 필터 로직)

### 2-2. 프리셋 저장 UI 개선
- 현재: [프리셋 저장]은 항상 프리셋0에 저장
- 목표: 프리셋 목록에서 선택 후 저장, 선택된 프리셋 하이라이트
- 관련: `SelectedPresetIndex` 상태 변수, P0~P9 클릭 시 선택 상태 업데이트, 저장 시 선택된 인덱스에 저장

### 2-3. 창 드래그 이동
- 현재: 창 위치 고정 (Canvas 픽셀 좌표)
- 목표: 타이틀바 드래그로 창 이동 가능
- 구현: `NativeOnMouseButtonDown`, `NativeOnMouseMove`, `NativeOnMouseButtonUp` 오버라이드

### 2-4. 모션 목록 10개 이상 지원
- 현재: L0~L9, B0~B9 UFUNCTION만 존재 → 최대 10개
- 목표: 동적 델리게이트 바인딩으로 제한 없이 지원
- 구현: `FScriptDelegate` + `FDynamicDelegate` 대신 인덱스를 캡처하는 방식 (람다 또는 별도 UObject 서브오브젝트)

### 2-5. 탭 필터 + 타입 일치 수정
- 현재: Item=의상모션, Social=제스처 (임의 매핑)
- 목표: `ECustomMotionType::Costume`, `ECustomMotionType::Gesture` 로 enum 확장 또는 기존 타입에 맞게 재정의
- 혹은: 탭과 관계없이 모든 모션 표시 + 탭은 카테고리 태그 역할

## 파일 구조 현황
```
Source/AITestProject/CustomizingMotion/
├── CustomizingMotionTypes.h        ← 타입/상수 정의
├── CustomizingMotionComponent.h/cpp ← 게임플레이 로직
├── CustomizingMotionWidget.h/cpp   ← UI (Step 1 재작성 완료)
└── NEXT_STEP_PROMPT.md             ← 이 파일
```

## 컴파일 전 체크리스트
- [ ] UE5.7 기준: `UButton::GetStyle()/SetStyle()` 사용 ✅
- [ ] `UBorder::SetPadding()` 사용 ✅
- [ ] `UVerticalBoxSlot::SetSize(FSlateChildSize)` 사용 ✅
- [ ] `UCanvasPanelSlot::SetOffsets(FMargin)` + `SetAnchors(FAnchors)` 사용 ✅
- [ ] `#include "Components/VerticalBoxSlot.h"` / `HorizontalBoxSlot.h"` 포함 ✅
- [ ] ABP에 `DefaultSlot` 노드 추가 필요 (AnimSequence 재생용) ⚠️
