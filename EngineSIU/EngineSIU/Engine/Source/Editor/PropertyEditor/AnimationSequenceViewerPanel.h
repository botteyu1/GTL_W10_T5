#pragma once

#include "Engine/EditorEngine.h"
#include "UnrealEd/EditorPanel.h"
#include <ImGui/ImSequencer.h>

#include "Animation/AnimSequence.h"


class UAnimSequence;
// ImSequencer를 사용하기 위한 간단한 FSequenceInterface 구현체
// 실제로는 엔진의 애니메이션 데이터를 이 인터페이스에 맞게 제공해야 합니다.
class FSequenceInterface : public ImSequencer::SequenceInterface
{
public:
    UAnimSequence* CurrentSequence = nullptr;
    int SelectedEntry = -1; // 선택된 트랙 인덱스 (예시)

    FSequenceInterface(UAnimSequence* InSequence) : CurrentSequence(InSequence)
    {
        UpdateLabels();
    }
    FSequenceInterface() {}

    void UpdateLabels()
    {
        ItemLabels.clear();
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
        if (CurrentSequence)
        {
            // 초당 프레임 수와 재생 길이를 기반으로 총 프레임 수 계산
            return static_cast<int>(CurrentSequence->GetPlayLength() * CurrentSequence->GetFrameRate().AsDecimal());
        }
        return 100; // 기본값 또는 시퀀스가 없을 때의 값
    }

    virtual int GetItemCount() const override
    {
        if (CurrentSequence)
        {
            // 예시: 본 트랙의 수를 반환하거나, 표시할 주요 트랙(예: 루트 모션, 커브) 수를 반환

            const TArray<FBoneAnimationTrack>* Track  =  CurrentSequence->GetBoneAnimationTracks();
            if (Track)
            {
                return Track->Num(); // 본 트랙 수
            }
            
            return 0; 
        }
        return 0;
    }

    virtual const char* GetItemLabel(int index) const override
    {
        if (CurrentSequence)
        {
            const TArray<FBoneAnimationTrack>* tracksPtr = CurrentSequence->GetBoneAnimationTracks();
            if (!tracksPtr)
            {
                return ""; 
            }
            const TArray<FBoneAnimationTrack>& tracksArray = *tracksPtr; // 포인터를 역참조하여 배열 자체에 대한 참조를 얻음


            if (index >= 0 && index < tracksArray.Num())
            {
                return *tracksArray[index].Name.ToString();
            }
            else if (index == 0 && tracksArray.IsEmpty()) // 본 트랙이 없고 index가 0이면 "Master Track"
            {
                return "Master Track";
            }
        }
        return "";
    }

    virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        // 각 트랙의 시작/끝 프레임, 타입, 색상 등을 설정합니다.
        // 이 예제에서는 하나의 트랙이 전체 시퀀스 길이를 가진다고 가정합니다.
        if (CurrentSequence && index == 0)
        {
            // ImSequencer는 start/end를 int*로 받으므로, 멤버 변수나 static 변수를 사용해야 할 수 있습니다.
            // 여기서는 간단히 시퀀스 전체 길이를 사용하도록 합니다.
            // 실제로는 각 애니메이션 클립(세그먼트)의 시작/끝을 나타내야 합니다.
            static int sequenceStart = 0; // 실제로는 CurrentSequence->GetFrameMin() 또는 해당 트랙의 시작 프레임
            static int sequenceEnd = 0;   // 실제로는 CurrentSequence->GetFrameMax() 또는 해당 트랙의 끝 프레임

            sequenceStart = GetFrameMin();
            sequenceEnd = GetFrameMax();

            if (start) *start = &sequenceStart;
            if (end) *end = &sequenceEnd;
            if (type) *type = 0; // 트랙 타입 (사용자 정의)
            if (color) *color = 0xFFFFAA80; // 트랙 색상 (예: 연한 주황색)
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
    virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override {}
};



class AnimationSequenceViewerPanel : public UEditorPanel
{
public:
    AnimationSequenceViewerPanel();
    virtual ~AnimationSequenceViewerPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void SetAnimationSequence(UAnimSequence* InAnimSequence);

private:
    float Width = 0, Height = 0;
    UAnimSequence* CurrentAnimSequence = nullptr;
    FSequenceInterface* SequencerData = nullptr; // ImSequencer에 전달할 데이터

    // 타임라인 상태
    int CurrentFrame = 0;
    int FirstFrame = 0; // 타임라인 스크롤 시작 프레임
    bool bIsExpanded = true; // 시퀀서 확장 여부 (ImSequencer용)
    int SelectedSequencerEntry = -1; // ImSequencer에서 선택된 아이템

    // 재생 관련 상태
    bool bIsPlaying = false;
    float PlaybackTime = 0.0f; // 초 단위 재생 시간
    float LastFrameTime = 0.0f; // 마지막 프레임 시간 (델타 타임 계산용)

    void RenderTimelineControls(USkeletalMeshComponent* SkeletalMeshComp);
    void UpdatePlayback(float DeltaTime, USkeletalMeshComponent* SkeletalMeshComponent);
};
