#pragma once

#include "Engine/EditorEngine.h"
#include "UnrealEd/EditorPanel.h"
#include <ImGui/ImSequencer.h>

#include "Animation/AnimSequence.h"


class AnimationSequenceViewerPanel;
class UAnimSequence;
class UAnimSingleNodeInstance;
// ImSequencer를 사용하기 위한 간단한 FSequenceInterface 구현체
// 실제로는 엔진의 애니메이션 데이터를 이 인터페이스에 맞게 제공해야 합니다.
class FSequenceInterface : public ImSequencer::SequenceInterface
{
public:
    UAnimSequence* CurrentSequence = nullptr;
    AnimationSequenceViewerPanel* OwnerPanel = nullptr; // 컨텍스트를 위한 소유 패널 포인터
    int SelectedEntry = -1; // 선택된 트랙 인덱스 (예시)

    float FramePixelWidth = 10.0f; // 프레임 당 픽셀 너비
    int FirstFrame = 0;            // 타임라인 시작 프레임

    FSequenceInterface(UAnimSequence* InSequence, AnimationSequenceViewerPanel* InOwner)
        : CurrentSequence(InSequence), OwnerPanel(InOwner) 
    {
        UpdateLabels();
    }

    void UpdateLabels()
    {
        ItemLabels.clear();
        //IMGUI를 위한 char 임시 버퍼 업데이트 
        if (CurrentSequence)
        {
            const TArray<FBoneAnimationTrack>* tracksPtr = CurrentSequence->GetBoneAnimationTracks();
            if (tracksPtr)
            {
                const TArray<FBoneAnimationTrack>& tracksArray = *tracksPtr;
                for (const auto& track : tracksArray)
                {
                    ItemLabels.push_back((*track.Name.ToString()));
                }
            }
            if (ItemLabels.empty()) // 본 트랙이 없으면 "Master Track"
            {
                ItemLabels.push_back("Master Track");
            }
        }
    }


    virtual int GetFrameMin() const override
    {
        return 0; // 실제 시퀀스의 시작 프레임
    }

    virtual int GetFrameMax() const override
    {
        if (CurrentSequence && CurrentSequence->GetFrameRate().IsValid())
        {
            // 초당 프레임 수와 재생 길이를 기반으로 총 프레임 수 계산
            return static_cast<int>(CurrentSequence->GetPlayLength() * CurrentSequence->GetFrameRate().AsDecimal());
        }
        return 100; // 기본값 또는 시퀀스가 없을 때의 값
    }

    // 현재는 노티파이를 위한 하나의 전용 트랙만 가짐
    virtual int GetItemCount() const override { return 1; }

    virtual const char* GetItemLabel(int index) const override
    {
        if (index == 0) return "Anim Notifies"; // 노티파이 트랙 레이블
        return "";
    }
    
    // 이 함수는 트랙 자체의 시각적 범위를 정의함
    virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        if (index == 0) // 노티파이 트랙
        {
            // 시작/끝 프레임은 int*로 전달되므로 static 변수 사용 (주의)
            static int sequenceStart = 0;
            static int sequenceEnd = 0;
            sequenceStart = GetFrameMin();
            sequenceEnd = GetFrameMax();

            if (start) *start = &sequenceStart; // 시작 프레임 포인터 설정
            if (end) *end = &sequenceEnd;     // 끝 프레임 포인터 설정
            if (type) *type = 1;              // 노티파이 트랙 타입 지정 (사용자 정의)
            if (color) *color = 0xFF6A4A89;   // 트랙 색상 지정 (예: 보라색)
        }
    }

    std::vector<std::string> ItemLabels; // imgui에 띄울 트랙 이름을 저장할 벡터

    virtual void BeginEdit(int index) override {}
    virtual void EndEdit() override {}
    virtual int GetItemTypeCount() const override { return 1; } // 예: "Bone Track", "Curve Track"
    virtual const char* GetItemTypeName(int typeIndex) const override { return "Anim Track"; }
    virtual void Add(int type) override {}
    virtual void Del(int index) override {}
    virtual void Duplicate(int index) override {}
    virtual size_t GetCustomHeight(int index) override { return 0; } // 트랙별 커스텀 높이
    virtual void DoubleClick(int index) override { SelectedEntry = index; }
    virtual void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect) override {}
    virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override ;

private:
};

class AnimationSequenceViewerPanel : public UEditorPanel
{
public:
    AnimationSequenceViewerPanel();
    virtual ~AnimationSequenceViewerPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void SetAnimationSequence(UAnimSequence* InAnimSequence);

    // --- 노티파이 선택 관련 ---
    int GetSelectedNotifyIndex() const { return SelectedNotifyIndex; } // 선택된 노티파이 인덱스 반환
    void SetSelectedNotifyIndex(int index) { SelectedNotifyIndex = index; } // 선택된 노티파이 인덱스 설정

    // --- 노티파이 수정 요청 관련 ---
    void RequestAddNotifyAtTime(float time); // MySequence가 더블클릭 시 호출
    void AddNewNotify(FName notifyName, float triggerTime); // 실제로 노티파이를 추가하는 함수
    void DeleteSelectedNotify(); // 선택된 노티파이 삭제 함수
    
    int DraggingNotifyIndex = -1; // 현재 드래그 중인 노티파이의 인덱스 (-1이면 드래그 중 아님)
    float DraggingNotifyOriginalTime = 0.0f; // 드래그 시작 시 노티파이의 원래 시간


private:
    float Width = 0, Height = 0;
    UAnimSequence* CurrentAnimSequence = nullptr;
    FSequenceInterface* SequencerData = nullptr; // ImSequencer에 전달할 데이터
    UAnimSingleNodeInstance* AnimSingleNodeInstance = nullptr; // 싱글노드 애니메이션 인스턴스

    

    

    // 타임라인 상태
    int CurrentFrame = 0;
    int FirstFrame = 0; // 타임라인 스크롤 시작 프레임
    bool bIsExpanded = false; // 시퀀서 확장 여부 (ImSequencer용)
    int SelectedSequencerEntry = -1; // ImSequencer에서 선택된
    

    // 재생 관련 상태
    bool bIsPlaying = false;
    float PlaybackTime = 0.0f; // 초 단위 재생 시간
    float LastFrameTime = 0.0f; // 마지막 프레임 시간 (델타 타임 계산용)

    bool bLooping = false; // 반복 재생 상태
    bool bIsPlayingReverse = false; // 역재생 상태
    float FrameStepTime = 0.0f; // 다음/이전 프레임 이동 시 사용할 시간 (1프레임에 해당하는 시간)

    void RenderTimelineControls(USkeletalMeshComponent* SkeletalMeshComp);
    void UpdatePlayback(float DeltaTime, USkeletalMeshComponent* SkeletalMeshComponent);

    int SelectedNotifyIndex = -1; // CurrentSequence->Notifies 배열 내의 선택된 인덱스

    // --- 노티파이 추가 팝업을 위한 임시 상태 ---
    bool bShowAddNotifyPopup = false;     // 팝업 표시 여부
    float AddNotifyTimeRequest = 0.0f;    // 추가 요청된 시간
    char AddNotifyNameBuffer[128] = "NewNotify"; // 팝업에서 사용할 이름 버퍼

    void RenderNotifyControls();   // 노티파이 추가/삭제 버튼 렌더링 함수 (신규)
    void RenderAddNotifyPopup();   // 노티파이 추가 팝업 렌더링 함수 (신규)
    void ProcessNotifiesInRange(float StartTime, float EndTime, USkeletalMeshComponent* SkeletalMeshComponent); // 재생 중 노티파이 트리거 함수 (유지)

    TSet<FName> TriggeredNotifyNamesThisFrame; // 한 프레임에 동일 노티파이 중복 실행 방지용 (선택적)
    int LastProcessedFrameForNotifies = -1; // 노티파이 처리를 위한 마지막 프레임
};
