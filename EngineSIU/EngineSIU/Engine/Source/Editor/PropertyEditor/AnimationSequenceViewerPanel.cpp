#include "AnimationSequenceViewerPanel.h"
#include <chrono>
 #include "Engine/EditorEngine.h" // UEditorEngine 접근용
 #include "Engine/Classes/Animation/AnimSequence.h" // UAnimSequence 사용
#include "Engine/Source/Editor/UnrealEd/IconsFeather.h" // Feather Icons 헤더 (또는 사용 중인 아이콘 폰트 헤더)
#include "Font/IconDefs.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "ImGui/imgui_internal.h"


void FSequenceInterface::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
{
    // 노티파이 트랙(index 0)에 대해서만 그림
    if (!CurrentSequence || index != 0 || !OwnerPanel)
    {
        return;
    }

    int frameMin = GetFrameMin();
    int frameMax = GetFrameMax();
    int totalFrames = frameMax - frameMin;
    if (totalFrames <= 0) return; // 프레임 수가 유효하지 않으면 그리지 않음

    float frameRate = CurrentSequence->GetFrameRate().AsDecimal();
    if (frameRate <= 0.0f) return;
    float markerSize = 12.0f; // 다이아몬드 마커 크기

    // rc의 너비는 현재 줌 레벨에서 전체 프레임(totalFrames)이 차지하는 총 픽셀 너비와 같습니다.
    float totalPixelWidth = rc.Max.x - rc.Min.x - markerSize;

    // 따라서 현재 프레임 당 픽셀 너비(framePixelWidth)는 다음과 같이 계산할 수 있습니다.
    // (totalFrames가 0일 경우 대비)
    float framePixelWidth = (totalFrames > 0) ? (totalPixelWidth / (float)totalFrames) : 10.0f;

    // rc.Min.x는 frameMin 프레임이 현재 화면에 그려지는 x 좌표입니다.

    // 마커가 클리핑 영역(실제로 보이는 영역) 내에 있는지 확인합니다.
    // Y 좌표는 트랙의 상단을 기준으로 합니다 (rc.Min.y ).
    float markerPosY = rc.Min.y + markerSize / 2.f; // 트랙 상단에 그리기
    
    float startScreenX = rc.Min.x; // 시작점의 화면 X 좌표
    ImU32 markerColor = IM_COL32(255, 255, 0, 255); // 노란색 마커
    ImU32 selectedMarkerColor = IM_COL32(0, 255, 0, 255); // 녹색 선택된 마커

    

    // 시퀀스의 모든 노티파이 순회
    for (int notifyIndex = 0; notifyIndex < CurrentSequence->GetAnimNotifies().Num(); ++notifyIndex)
    {
        const FAnimNotifyEvent& notify = CurrentSequence->GetAnimNotifies()[notifyIndex];
        int notifyFrame = static_cast<int>(notify.TriggerTime * frameRate);
        // notifyFrame이 화면에 그려져야 할 x 좌표 계산:
        // 시작점(frameMin의 화면 X 좌표) + (프레임 차이 * 프레임 당 픽셀 너비)
        float markerScreenX = startScreenX + (notifyFrame - frameMin) * framePixelWidth;



        // X 좌표가 보이는 영역 내에 있는지 확인
        if (markerScreenX >= clippingRect.Min.x && markerScreenX <= clippingRect.Max.x)
        {
            // 마커 포인트 계산 (markerScreenX를 중심으로, 상단 가장자리는 rc.Min.y에 위치하는 다이아몬드 모양)
            // 간단한 다이아몬드 모양 마커 그리기

            
            ImVec2 p1 = ImVec2(markerScreenX, markerPosY);
            ImVec2 p2 = ImVec2(markerScreenX + markerSize / 2, markerPosY + markerSize / 2);
            ImVec2 p3 = ImVec2(markerScreenX, markerPosY + markerSize);
            ImVec2 p4 = ImVec2(markerScreenX - markerSize / 2, markerPosY + markerSize / 2);

            // 선택 상태에 따라 색상 결정 (OwnerPanel이 관리)
            bool isSelected = (OwnerPanel->GetSelectedNotifyIndex() == notifyIndex);
            ImU32 currentColor = isSelected ? selectedMarkerColor : markerColor;

            // 채워진 사각형(다이아몬드) 그리기
            draw_list->AddQuadFilled(p1, p2, p3, p4, currentColor);

            // --- 상호작용 ---
            ImRect markerHitbox(p4, p2); // 상호작용을 위한 경계 상자
            markerHitbox.Min.y = rc.Min.y; // 클릭하기 쉽도록 히트박스를 트랙 전체 높이로 설정
            markerHitbox.Max.y = rc.Max.y;

            // 마우스 오버 시 툴팁으로 노티파이 이름 표시
            ImRect markerRect(ImVec2(markerScreenX - markerSize / 2, markerPosY), ImVec2(markerScreenX + markerSize / 2, markerPosY + markerSize));


            
            if (markerRect.Contains(ImGui::GetMousePos()))
            {
                ImGui::BeginTooltip();
                ImGui::Text("Notify: %s", (*notify.NotifyName.ToString()));
                ImGui::Text("Time: %.2fs (Frame: %d)", notify.TriggerTime, notifyFrame);
                ImGui::EndTooltip();

                // 클릭 시 선택
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    OwnerPanel->DraggingNotifyIndex = notifyIndex; // 드래그 시작
                    OwnerPanel->DraggingNotifyOriginalTime = notify.TriggerTime;
                    // TODO: 선택된 노티파이 정보 업데이트 (예: PropertyEditor에 표시)
                    // OwnerPanel->SelectNotify(¬ify);
                    
                    OwnerPanel->SetSelectedNotifyIndex(notifyIndex); // 패널에 선택된 인덱스 알림
                    //ImGui::CaptureMouseFromApp(true); // 다른 상호작용 방지
                    ImGui::SetNextFrameWantCaptureMouse(true); //대신 사용
                }
            }
        }
    }

    // 빈 공간 클릭 시 선택 해제 처리
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && clippingRect.Contains(ImGui::GetMousePos()))
    {
        bool clickedOnMarker = false;
        // 마우스 위치가 기존 마커 위에 있는지 확인
        for (int notifyIndex = 0; notifyIndex < CurrentSequence->GetAnimNotifies().Num(); ++notifyIndex)
        {
            const FAnimNotifyEvent& notify = CurrentSequence->GetAnimNotifies()[notifyIndex];
            int notifyFrame = static_cast<int>(notify.TriggerTime * frameRate);
            float markerScreenX = startScreenX + (notifyFrame - frameMin) * framePixelWidth;
            ImRect markerHitbox(
                ImVec2(markerScreenX - markerSize / 2, rc.Min.y),
                ImVec2(markerScreenX + markerSize / 2, rc.Max.y)
            );
            if (markerHitbox.Contains(ImGui::GetMousePos())) {
                clickedOnMarker = true;
                break;
            }
        }
        // 마커 위가 아닌 빈 공간을 클릭했다면 선택 해제
        if (!clickedOnMarker) {
            OwnerPanel->SetSelectedNotifyIndex(-1);
        }
    }

    // --- 드래그 중인 노티파이 위치 업데이트 ---
    if (OwnerPanel && OwnerPanel->DraggingNotifyIndex != -1)
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) // 임계값 없이 드래그 감지
        {
            if (OwnerPanel->DraggingNotifyIndex >= 0 && OwnerPanel->DraggingNotifyIndex < CurrentSequence->GetAnimNotifies().Num())
            {
                FAnimNotifyEvent& draggedNotify = CurrentSequence->GetAnimNotifies()[OwnerPanel->DraggingNotifyIndex];
                float mouseX = ImGui::GetMousePos().x;

                // 화면 X 좌표를 시간으로 변환 (더블 클릭 로직과 유사)
                if (framePixelWidth > 0.0f)
                {
                    float relativeClickX = mouseX - rc.Min.x;
                    int newFrame = frameMin + static_cast<int>(relativeClickX / framePixelWidth);
                    newFrame = std::max(frameMin, std::min(newFrame, frameMax)); // 프레임 범위 제한

                    float newTime = static_cast<float>(newFrame) / frameRate;
                    newTime = std::max(0.0f, std::min(newTime, CurrentSequence->GetPlayLength())); // 시간 범위 제한

                    draggedNotify.TriggerTime = newTime; // 노티파이 시간 직접 업데이트

                    // TODO: 변경 사항을 즉시 반영하거나, 에셋을 더티 상태로 표시
                    // CurrentSequence->MarkPackageDirty();
                }
            }
        }
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            OwnerPanel->DraggingNotifyIndex = -1; // 드래그 종료
        }
    }



     // 더블 클릭으로 해당 시간에 새 노티파이 추가 요청
    // --- 더블 클릭으로 새 노티파이 추가 요청 ---
    if (OwnerPanel && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && clippingRect.Contains(ImGui::GetMousePos()))
    {
        bool clickedOnExistingMarker = false;
        float mouseX = ImGui::GetMousePos().x;

        if (CurrentSequence && CurrentSequence->GetFrameRate().AsDecimal() > 0.0f)
        {
            int frameMin = GetFrameMin();
            int totalFrames = GetFrameMax() - frameMin;
            float currentFramePixelWidth = (totalFrames > 0) ? ((rc.Max.x - rc.Min.x) / (float)totalFrames) : 10.0f;
            float startScreenX = rc.Min.x;

            for (const FAnimNotifyEvent& notify : CurrentSequence->GetAnimNotifies())
            {
                int notifyFrame = static_cast<int>(notify.TriggerTime * CurrentSequence->GetFrameRate().AsDecimal());
                float markerScreenX = startScreenX + (notifyFrame - frameMin) * currentFramePixelWidth;
                ImRect markerRect(ImVec2(markerScreenX - markerSize / 2, markerPosY), ImVec2(markerScreenX + markerSize / 2, markerPosY + markerSize));
            
                //ImRect markerRect(ImVec2(markerScreenX - markerSize, rc.Min.y - markerSize), ImVec2(markerScreenX + markerSize, rc.Min.y + markerSize + markerSize)); // 클릭 영역을 좀 더 넓게
                if (markerRect.Contains(ImGui::GetMousePos()))
                {
                    clickedOnExistingMarker = true;
                    // TODO: 기존 노티파이 편집 로직 호출
                    break;
                }
            }
        }

        if (!clickedOnExistingMarker) // 빈 공간을 더블 클릭한 경우
        {
            // 화면 X 좌표를 프레임으로 변환
            int frameMin = GetFrameMin();
            int totalFrames = GetFrameMax() - frameMin;
            if (totalFrames <= 0) return; // 유효하지 않으면 중단

            // rc.Min.x는 frameMin 프레임이 현재 화면에 그려지는 x 좌표.
            // rc.Max.x는 frameMax 프레임이 현재 화면에 그려지는 x 좌표.
            float totalPixelWidthForSequence = rc.Max.x - rc.Min.x;
            float currentFramePixelWidth = totalPixelWidthForSequence / (float)totalFrames;

            if (currentFramePixelWidth > 0.0f)
            {
                // 클릭된 X 좌표(mouseX)에서 트랙 시작 X 좌표(rc.Min.x)를 빼서
                // 트랙 내에서의 상대적인 X 픽셀 위치를 구함.
                float relativeClickX = mouseX - rc.Min.x;

                // 상대적인 X 픽셀 위치를 프레임으로 변환하고, 시퀀스 시작 프레임(frameMin)을 더함.
                int clickedFrame = frameMin + static_cast<int>(relativeClickX / currentFramePixelWidth);

                // 계산된 프레임이 유효한 범위 내에 있는지 확인
                clickedFrame = std::max(frameMin, std::min(clickedFrame, GetFrameMax()));

                if (CurrentSequence && CurrentSequence->GetFrameRate().IsValid())
                {
                    // 프레임을 시간으로 변환
                    float clickedTime = static_cast<float>(clickedFrame) / CurrentSequence->GetFrameRate().AsDecimal();

                    // 시간을 시퀀스 길이 내로 최종 제한 (부동소수점 오차 등 고려)
                    clickedTime = std::max(0.0f, std::min(clickedTime, CurrentSequence->GetPlayLength()));

                    // 패널에 해당 시간에 노티파이 추가 요청
                    OwnerPanel->RequestAddNotifyAtTime(clickedTime);
                }
            }
        }
    }
}

AnimationSequenceViewerPanel::AnimationSequenceViewerPanel()
 {
     // 이 패널이 어떤 월드 타입에서 활성화될지 설정 (예: SkeletalViewer 전용)
    //EWorldTypeBitFlag::Editor|EWorldTypeBitFlag::PIE|
     SetSupportedWorldTypes(EWorldTypeBitFlag::SkeletalViewer);
     // 초기화 시 LastFrameTime 설정
     LastFrameTime = static_cast<float>(ImGui::GetTime());
 }

 AnimationSequenceViewerPanel::~AnimationSequenceViewerPanel()
 {
     delete SequencerData; // 동적 할당된 경우 해제
 }

 void AnimationSequenceViewerPanel::SetAnimationSequence(UAnimSequence* InAnimSequence)
 {
    CurrentAnimSequence = InAnimSequence; // 현재 애니메이션 시퀀스 설정
    // 상태 초기화
    PlaybackTime = 0.0f;
    CurrentFrame = 0;
    FirstFrame = 0;
    bIsPlaying = false;
    bIsPlayingReverse = false;
    SelectedNotifyIndex = -1; // 선택 해제
    bShowAddNotifyPopup = false; // 팝업 닫기
    
    DraggingNotifyIndex = -1; // 시퀀스 변경 시 드래그 상태 초기화

    delete SequencerData; // 기존 SequencerData 삭제
    if (CurrentAnimSequence)
    {
        // MySequence 생성 시 'this' 포인터 전달
        SequencerData = new FSequenceInterface(CurrentAnimSequence, this);
    }
    else
    {
        SequencerData = nullptr;
    }
 }

void AnimationSequenceViewerPanel::RequestAddNotifyAtTime(float time)
{
    if (!CurrentAnimSequence) return; // 현재 시퀀스 없으면 중단
    // 요청 시간을 시퀀스 길이 내로 제한
    AddNotifyTimeRequest = std::max(0.0f, std::min(time, CurrentAnimSequence->GetPlayLength()));
    bShowAddNotifyPopup = true; // 팝업 표시 플래그 설정
    //ImGui::OpenPopup("Add New Notify"); // 이름으로 팝업 열기
}

void AnimationSequenceViewerPanel::AddNewNotify(FName notifyName, float triggerTime)
{
    if (!CurrentAnimSequence) return; // 현재 시퀀스 없으면 중단

    // 새 FAnimNotifyEvent 객체 생성 및 설정
    FAnimNotifyEvent newNotify;
    newNotify.NotifyName = notifyName;
    newNotify.TriggerTime = triggerTime;
    newNotify.Duration = 0.0f; // 기본 지속 시간 설정

    // 현재 시퀀스의 Notifies 배열에 새 노티파이 추가
    CurrentAnimSequence->GetAnimNotifies().Add(newNotify);

    // 시간순으로 노티파이 정렬 
    CurrentAnimSequence->GetAnimNotifies().Sort([](const FAnimNotifyEvent& A, const FAnimNotifyEvent& B) {
        return A.TriggerTime < B.TriggerTime;
    });

    // TODO: UAnimSequence 에셋이 수정되었음을 표시하여 저장 가능하도록 함
    // CurrentAnimSequence->MarkPackageDirty();
}

void AnimationSequenceViewerPanel::DeleteSelectedNotify()
{
    // 현재 시퀀스가 없거나 유효한 인덱스가 아니면 중단
    if (!CurrentAnimSequence || SelectedNotifyIndex < 0 || SelectedNotifyIndex >= CurrentAnimSequence->GetAnimNotifies().Num())
    {
        return;
    }

    // Notifies 배열에서 선택된 인덱스의 노티파이 제거
    CurrentAnimSequence->GetAnimNotifies().RemoveAt(SelectedNotifyIndex);
    SelectedNotifyIndex = -1; // 삭제 후 선택 해제

    // TODO: UAnimSequence 에셋이 수정되었음을 표시
    // CurrentAnimSequence->MarkPackageDirty();
}

void AnimationSequenceViewerPanel::Render()
 {
     UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
     USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Engine->GetSelectedComponent());

     // 전 애니메이션과 달라지면 업데이트
     // AnimInstance가 SingleNodeInstance인 경우에만 애니메이션을 업데이트 -> 뷰어니깐
     if (Engine and SkeletalMeshComponent)
     {
         AnimSingleNodeInstance = Cast<UAnimSingleNodeInstance>(SkeletalMeshComponent->GetAnimInstance());

         static UAnimSequence* PrevAnim = nullptr;
         if (AnimSingleNodeInstance && PrevAnim != AnimSingleNodeInstance->GetAnimationAsset())
         {
             if (UAnimationAsset* AnimAsset = AnimSingleNodeInstance->GetAnimationAsset())
             {
                 CurrentAnimSequence = Cast<UAnimSequence>(AnimAsset); // !TODO : 현 시점에서는 UAnimSequence뿐이니까 이렇게 캐스팅
                SetAnimationSequence(CurrentAnimSequence);
             }
         }
         PrevAnim = CurrentAnimSequence;
     }
     if (!Engine || !CurrentAnimSequence || !SequencerData || !SkeletalMeshComponent || !AnimSingleNodeInstance) // 엔진 또는 시퀀스 없으면 렌더링 안 함
     {
         // 패널은 표시하되, 내용이 없음을 알릴 수 있음
         ImGui::Begin("Animation Sequence Viewer", nullptr, ImGuiWindowFlags_NoCollapse);
         ImGui::Text("No Animation Sequence loaded.");
         ImGui::End();
         return;
     }

     // 현재 시간 가져오기 및 델타 타임 계산
     float currentTime = static_cast<float>(ImGui::GetTime());
     float deltaTime = currentTime - LastFrameTime;
     LastFrameTime = currentTime;

    // --- 팝업 열기 요청 처리 ---
    // bShowAddNotifyPopup 플래그가 true이고, 해당 팝업이 아직 열려있지 않다면 OpenPopup 호출
    if (bShowAddNotifyPopup && !ImGui::IsPopupOpen("Add New Notify", ImGuiPopupFlags_AnyPopupId)) {
        ImGui::OpenPopup("Add New Notify");
    }
    // 노티파이 추가 팝업 렌더링 (요청된 경우)
    RenderAddNotifyPopup();

    UpdatePlayback(deltaTime, SkeletalMeshComponent);
     

     // --- 패널 레이아웃 설정 (PropertyEditorPanel.cpp 참고) ---
     // 예시: 화면 하단에 길게 배치
     float PanelWidth = Width; // 전체 너비 사용
     float PanelHeight = Height * 0.25f; // 화면 높이의 25% 사용
     float PanelPosX = 0.0f;
     float PanelPosY = Height - PanelHeight; // 화면 하단

     ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);
     ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

     ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_MenuBar| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ;
     // ImGuiWindowFlags_MenuBar 플래그를 추가하여 메뉴바 공간 확보 가능

     ImGui::Begin("Animation Sequence Viewer", nullptr, PanelFlags);

     // 1. 재생 컨트롤 UI (버튼 등)
     RenderTimelineControls(SkeletalMeshComponent);
     ImGui::Separator();

   // 노티파이 추가/삭제 컨트롤 UI
    RenderNotifyControls(); // 이 함수 호출 추가
    ImGui::Separator(); // 구분선

    // --- MySequence에 컨텍스트 정보 업데이트 ---
    // 이 부분이 까다로움. Sequencer 호출 전에 framePixelWidth와 firstFrame이 필요하지만,
    // 이 값들은 Sequencer 내부에서 계산됨.
    // 해결책: 이전 프레임 값을 사용하거나 추정.
    // 더 견고한 해결책은 ImSequencer를 수정하거나 상태 저장소를 조심스럽게 사용하는 것.
    // 일단은 패널의 FirstFrame을 전달하고 FramePixelWidth는 추정.
    SequencerData->FirstFrame = FirstFrame; // 패널의 FirstFrame 전달
    if (SequencerData->GetFrameMax() > SequencerData->GetFrameMin()) {
        // 사용 가능한 너비를 기반으로 추정 (대략적)
        float availableWidth = ImGui::GetContentRegionAvail().x - 200; // 대략적인 레전드 너비 제외
        SequencerData->FramePixelWidth = availableWidth / (float)(SequencerData->GetFrameMax() - SequencerData->GetFrameMin() + 1);
        SequencerData->FramePixelWidth = std::max(0.1f, SequencerData->FramePixelWidth); // 0 또는 음수 방지
    } else {
        SequencerData->FramePixelWidth = 10.0f; // 기본값
    }

     // 2. ImSequencer UI 호출
     // ImSequencer::Sequencer() 함수는 int* currentFrame을 받으므로, 멤버 변수 CurrentFrame을 사용합니다.
     // FirstFrame은 타임라인의 스크롤 위치를 제어합니다.
     // SelectedSequencerEntry는 시퀀서 내에서 아이템 선택 시 업데이트됩니다.
     int sequenceOptions = ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME; 
     
     // CurrentFrame을 시퀀서에 전달하기 전에 PlaybackTime 기준으로 업데이트
     if (CurrentAnimSequence && CurrentAnimSequence->GetFrameRate().AsDecimal() > 0.0f)
     {
         CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
         // CurrentFrame이 시퀀스 범위를 벗어나지 않도록 클램핑
         CurrentFrame = std::max(SequencerData->GetFrameMin(), std::min(CurrentFrame, SequencerData->GetFrameMax()));
     }

    // Sequencer 호출 전 FirstFrame 값 저장 (Sequencer가 변경할 수 있으므로)
    int firstFrameBefore = FirstFrame;

     if (ImSequencer::Sequencer(SequencerData, &CurrentFrame, &bIsExpanded, &SelectedSequencerEntry, &FirstFrame, sequenceOptions))
     {
          //시퀀서에서 프레임이 변경된 경우 (예: 사용자가 재생 헤드를 드래그)
          if (CurrentAnimSequence && CurrentAnimSequence->GetFrameRate().AsDecimal() > 0.0f)
          {
              PlaybackTime = static_cast<float>(CurrentFrame) / CurrentAnimSequence->GetFrameRate().AsDecimal();
              // TODO: 이 시간에 맞춰 스켈레탈 메쉬의 포즈를 업데이트하는 로직 호출

              SkeletalMeshComponent->SetAnimationTime(PlaybackTime); // USkeletalMeshComponent에 해당 함수 추가 필요
              
          }
     }
     // 사용자가 ImSequencer의 재생 헤드를 드래그한 경우 (드래그 종료 시점 확인)
     if (ImGui::IsItemDeactivated() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) == false)
     {
         // 프레임 속도가 유효하면
         if (CurrentAnimSequence && CurrentAnimSequence->GetFrameRate().IsValid())
         {
             // 변경된 CurrentFrame을 기반으로 PlaybackTime 업데이트
             PlaybackTime = static_cast<float>(CurrentFrame) / CurrentAnimSequence->GetFrameRate().AsDecimal();
             // 스켈레탈 메쉬 컴포넌트의 애니메이션 시간 업데이트

             SkeletalMeshComponent->SetAnimationTime(PlaybackTime);
             
             // 사용자가 수동으로 프레임을 설정했으므로 재생 중지
             bIsPlaying = false;
             bIsPlayingReverse = false;
         }
     }

    // 스크롤바 사용으로 FirstFrame이 변경된 경우
    if (FirstFrame != firstFrameBefore) {
        // 새로운 가시 범위에 따라 필요한 업데이트 수행 (예: 다시 그리기)
    }


    
     // static char NotifyName[256] = "";
     // if (CurrentAnimSequence)
     // {
     //     ImGui::InputText("NotifyName", NotifyName, sizeof(NotifyName));
     //
     //     if (ImGui::Button("Add Notify"))
     //     {
     //         FAnimNotifyEvent NewNotify;
     //         NewNotify.NotifyName = FName(FString(NotifyName));
     //         NewNotify.TriggerTime = PlaybackTime;
     //         NewNotify.Duration = 0.0f;
     //         CurrentAnimSequence->AddAnimNotifyEvent(NewNotify);
     //
     //         NotifyName[0] = '\0';
     //     }
     // }
     //
     // if(CurrentAnimSequence->GetAnimNotifies().Num()>0)
     //     ImGui::Text("Edit Notify Trigger Time:");
     //
     // int NotifyIndex = 0;
     // //삭제할 Notify 담아둘 리스트
     // TArray<FAnimNotifyEvent> DeletedNotify;
     //
     // for(auto& Notify : CurrentAnimSequence->GetAnimNotifies())
     // {
     //     ImGui::PushID(NotifyIndex);
     //     float FrameRate = CurrentAnimSequence->GetFrameRate().AsDecimal();
     //     float TimeInSeconds = Notify.TriggerTime;
     //
     //     ImGui::Separator();
     //     ImGui::Text(*Notify.NotifyName.ToString());
     //     ImGui::SameLine();
     //     if (ImGui::SliderFloat("##TriggerTime", &TimeInSeconds, 0.0f, CurrentAnimSequence->GetPlayLength(), "%.2f s"))
     //     {
     //         Notify.TriggerTime = TimeInSeconds;
     //     }
     //     ImGui::SameLine();
     //     if (ImGui::Button("Delete"))
     //     {
     //         DeletedNotify.Add(Notify);
     //     }
     //     ImGui::PopID();
     //     ++NotifyIndex;
     // }
     // //삭제할 Notify 일괄 처리
     // for (auto& Notify : DeletedNotify)
     // {
     //     CurrentAnimSequence->RemoveAnimNotifyEvent(Notify);
     // }
     //
     // // 선택된 트랙 정보 표시 (예시)
     // if (SelectedSequencerEntry != -1)
     // {
     //     ImGui::Text("Selected Track: %s", SequencerData->GetItemLabel(SelectedSequencerEntry));
     // }

     ImGui::End();
 }

 void AnimationSequenceViewerPanel::RenderTimelineControls(USkeletalMeshComponent* SkeletalMeshComp)
 {

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine); // 엔진 접근
    // 아이콘 폰트 Push (ControlEditorPanel.cpp 참고)
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT]; // FEATHER_FONT 인덱스가 올바르다고 가정
    ImGui::PushFont(IconFont);


    // 처음으로 이동 버튼
    if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_SKIP_BACK))) // "|<"
    {
        PlaybackTime = 0.0f;
        CurrentFrame = 0;
        bIsPlaying = false;
        bIsPlayingReverse = false;
        if (SkeletalMeshComp) SkeletalMeshComp->SetAnimationTime(PlaybackTime);
    }
    ImGui::SameLine();

    // 이전 프레임 버튼
    if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_CHEVRON_LEFT))) // "<"
    {
        if (CurrentAnimSequence && SequencerData)
        {
            FrameStepTime = 1.0f / CurrentAnimSequence->GetFrameRate().AsDecimal();
            PlaybackTime -= FrameStepTime;
            if (PlaybackTime < 0.0f) PlaybackTime = 0.0f;
            CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
            CurrentFrame = std::max(0, CurrentFrame); // 음수 방지
            bIsPlaying = false;
            bIsPlayingReverse = false;
            if (SkeletalMeshComp) SkeletalMeshComp->SetAnimationTime(PlaybackTime);
        }
    }
    ImGui::SameLine();
    
    // 역재생 버튼 (토글 방식 또는 별도 버튼)
    // 여기서는 간단히 정지 상태일 때만 활성화되는 역재생 시작 버튼으로 구현
    if (!bIsPlaying && !bIsPlayingReverse)
    {
        // ICON_FEATHER_ROTATE_CCW 같은 아이콘 사용 가능, 여기서는 "RPlay" 텍스트로 대체
        if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_REWIND))) // "RPlay" (역재생 아이콘으로 변경 필요)
        {
            bIsPlayingReverse = true;
            bIsPlaying = false;
            LastFrameTime = static_cast<float>(ImGui::GetTime());
            // 만약 재생 시간이 처음에 도달해 있었다면 끝에서부터 역재생
            if (CurrentAnimSequence && PlaybackTime <= KINDA_SMALL_NUMBER)
            {
                PlaybackTime = CurrentAnimSequence->GetPlayLength();
                CurrentFrame = SequencerData ? SequencerData->GetFrameMax() : 0;
                if (SkeletalMeshComp) SkeletalMeshComp->SetAnimationTime(PlaybackTime);
            }
        }
        ImGui::SameLine();
    }


    // 재생/정지/역재생 버튼
    if (bIsPlaying) // 정방향 재생 중
    {
        if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_PAUSE))) // "Pause" (Stop 대신 Pause 아이콘 사용)
        {
            bIsPlaying = false;
        }
    }
    else if (bIsPlayingReverse) // 역방향 재생 중
    {
        if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_PAUSE))) // "Pause"
        {
            bIsPlayingReverse = false;
        }
    }
    else // 정지 상태
    {
        if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_PLAY))) // "Play"
        {
            bIsPlaying = true;
            bIsPlayingReverse = false;
            // 재생 시작 시 LastFrameTime을 현재 시간으로 리셋
            LastFrameTime = static_cast<float>(ImGui::GetTime());
            // 만약 재생 시간이 끝에 도달해 있었다면 처음부터 재생
            if (CurrentAnimSequence && PlaybackTime >= CurrentAnimSequence->GetPlayLength() - KINDA_SMALL_NUMBER)
            {
                PlaybackTime = 0.0f;
                CurrentFrame = 0;
                if (SkeletalMeshComp) SkeletalMeshComp->SetAnimationTime(PlaybackTime);
            }
        }
    }
    ImGui::SameLine();



    // 다음 프레임 버튼
    if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_CHEVRON_RIGHT))) // ">"
    {
        if (CurrentAnimSequence && SequencerData)
        {
            FrameStepTime = 1.0f / CurrentAnimSequence->GetFrameRate().AsDecimal();
            PlaybackTime += FrameStepTime;
            float sequenceLength = CurrentAnimSequence->GetPlayLength();
            if (PlaybackTime > sequenceLength) PlaybackTime = sequenceLength;
            CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
            CurrentFrame = std::min(CurrentFrame, SequencerData->GetFrameMax()); // 최대 프레임 초과 방지
            bIsPlaying = false;
            bIsPlayingReverse = false;
            if (SkeletalMeshComp) SkeletalMeshComp->SetAnimationTime(PlaybackTime);
        }
    }
    ImGui::SameLine();

    // 끝으로 이동 버튼
    if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_SKIP_FORWARD))) // ">|"
    {
        if (CurrentAnimSequence && SequencerData)
        {
            PlaybackTime = CurrentAnimSequence->GetPlayLength();
            CurrentFrame = SequencerData->GetFrameMax();
            bIsPlaying = false;
            bIsPlayingReverse = false;
            if (SkeletalMeshComp) SkeletalMeshComp->SetAnimationTime(PlaybackTime);
        }
    }
    ImGui::SameLine();

    // 반복 재생 토글 버튼
    // 선택되었을 때 다른 색상으로 표시 (예시)
     bool bPushedColors = false; // 색상을 푸시했는지 여부를 추적하는 플래그
    if (bLooping)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f)); // 활성화 색상
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.8f, 1.0f));
        bPushedColors = true; // 색상을 푸시했음을 표시
    }
     if (ImGui::Button(reinterpret_cast<const char*>(ICON_FEATHER_REPEAT))) // "Loop"
     {
         bLooping = !bLooping;
     }

     if (bPushedColors) // PushStyleColor가 호출되었다면 반드시 PopStyleColor 호출
     {
         ImGui::PopStyleColor(3);
     }
    ImGui::SameLine();


    // 현재 시간 / 전체 시간 표시
    if (CurrentAnimSequence && SequencerData)
    {
        // 아이콘 폰트 Pop 후 기본 폰트로 텍스트 표시
        ImGui::PopFont();
        ImGui::Text("%.2f / %.2f s", PlaybackTime, CurrentAnimSequence->GetPlayLength());
        ImGui::SameLine();
        ImGui::Text("Frame: %d / %d", CurrentFrame, SequencerData->GetFrameMax());
        ImGui::PushFont(IconFont); // 다시 아이콘 폰트 Push (만약 뒤에 더 아이콘 버튼이 있다면)
    }

    ImGui::PopFont(); // 아이콘 폰트 Pop

     // TODO: 슬라이더로 시간 직접 조절 (ImGui::SliderFloat)
     ImGui::PushItemWidth(-1); // 슬라이더가 남은 공간을 꽉 채우도록
     if (CurrentAnimSequence && CurrentAnimSequence->GetPlayLength() > 0)
     {
         if (ImGui::SliderFloat("##Timeline", &PlaybackTime, 0.0f, CurrentAnimSequence->GetPlayLength(), "%.2f s"))
         {
             bIsPlaying = false; // 슬라이더 조작 시 자동 재생 정지 
             CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
             SkeletalMeshComp->SetAnimationTime(PlaybackTime);
             // TODO: 스켈레탈 메쉬 포즈 업데이트
         }
     }
     ImGui::PopItemWidth();
 }

 void AnimationSequenceViewerPanel::UpdatePlayback(float DeltaTime, USkeletalMeshComponent* SkeletalMeshComponent)
 {
     if (!CurrentAnimSequence || (!bIsPlaying && !bIsPlayingReverse))
     {
         return;
     }

     float sequenceLength = CurrentAnimSequence->GetPlayLength();
     float frameRate = CurrentAnimSequence->GetFrameRate().AsDecimal();
     if (frameRate <= 0.0f) return; // 유효하지 않은 프레임 속도

     if (bIsPlaying) // 정방향 재생
     {
         PlaybackTime += DeltaTime;
         if (PlaybackTime >= sequenceLength)
         {
             if (bLooping)
             {
                 // 루프: 총 길이를 초과한 만큼 처음부터 다시 시작
                 PlaybackTime = fmodf(PlaybackTime, sequenceLength);
                 // 또는 PlaybackTime -= sequenceLength; (정확한 루프 시작점을 위해)
                 // PlaybackTime = 0.0f; // 간단하게 처음으로
             }
             else
             {
                 PlaybackTime = sequenceLength;
                 bIsPlaying = false; // 재생 중지
             }
         }
     }
    else if (bIsPlayingReverse) // 역방향 재생
    {
        PlaybackTime -= DeltaTime;
        if (PlaybackTime <= 0.0f)
        {
            if (bLooping)
            {
                // 루프: 0보다 작아진 만큼 끝에서부터 다시 시작
                PlaybackTime = sequenceLength - fmodf(-PlaybackTime, sequenceLength);
                // 또는 PlaybackTime += sequenceLength;
                // PlaybackTime = sequenceLength; // 간단하게 끝으로
            }
            else
            {
                PlaybackTime = 0.0f;
                bIsPlayingReverse = false; // 재생 중지
            }
        }
    }
    
    // --- Anim Notify 트리거 ---
    if (CurrentAnimSequence && frameRate > 0.0f)
    {
        int previousFrame = LastProcessedFrameForNotifies; // 이전 프레임 (또는 이전 시간 기반)
        int currentLogicFrame = static_cast<int>(PlaybackTime * frameRate); // 현재 논리적 프레임

        // 처음 재생 시작 시 또는 시간이 뒤로 갔을 때 LastProcessedFrameForNotifies 초기화
        if (bIsPlaying && LastProcessedFrameForNotifies == -1) { // 재생 시작 시
             LastProcessedFrameForNotifies = currentLogicFrame;
        } else if (!bIsPlaying && !bIsPlayingReverse) { // 정지 시
            // 사용자가 타임라인을 직접 조작하여 시간이 변경된 경우에도 처리할 수 있도록.
            // 이 경우, 이전 프레임과 현재 프레임 사이의 모든 노티파이를 찾아야 함.
            // 여기서는 단순화를 위해 재생 중에만 처리.
        }


        // 시간 범위 결정 (이전 프레임과 현재 프레임 사이)
        // 루프 및 역재생 고려 필요
        float startTime, endTime;
        if (currentLogicFrame > previousFrame) // 정방향
        {
            startTime = static_cast<float>(previousFrame) / frameRate;
            endTime = PlaybackTime;
        }
        else if (currentLogicFrame < previousFrame) // 역방향 또는 루프 후 처음으로
        {
            // 역재생 시에는 endTime이 더 작음
            startTime = PlaybackTime;
            endTime = static_cast<float>(previousFrame) / frameRate;
        }
        else // 같은 프레임
        {
            startTime = endTime = PlaybackTime; // 또는 처리 안 함
        }
        
        // 루프 시 경계 처리: 예) 끝에서 처음으로 넘어갈 때
        if (bLooping && previousFrame > currentLogicFrame && bIsPlaying) { // 정방향 루프 발생
            // 1. previousFrame -> sequenceEnd 까지의 노티파이
            ProcessNotifiesInRange(static_cast<float>(previousFrame) / frameRate, sequenceLength, SkeletalMeshComponent);
            // 2. 0 -> currentLogicFrame 까지의 노티파이
            ProcessNotifiesInRange(0.0f, PlaybackTime, SkeletalMeshComponent);
        } else if (bLooping && previousFrame < currentLogicFrame && bIsPlayingReverse) { // 역방향 루프 발생
            // 1. previousFrame -> 0 까지의 노티파이
            ProcessNotifiesInRange(static_cast<float>(previousFrame) / frameRate, 0.0f, SkeletalMeshComponent);
            // 2. sequenceEnd -> currentLogicFrame 까지의 노티파이
            ProcessNotifiesInRange(sequenceLength, PlaybackTime, SkeletalMeshComponent);
        }
        else { // 일반적인 경우 또는 루프 아닌 경우
            ProcessNotifiesInRange(startTime, endTime, SkeletalMeshComponent);
        }


        LastProcessedFrameForNotifies = currentLogicFrame;
    }


     // 현재 프레임 업데이트 (ImSequencer는 CurrentFrame을 직접 사용)
     CurrentFrame = static_cast<int>(PlaybackTime * frameRate);
     CurrentFrame = std::max(0, std::min(CurrentFrame, SequencerData ? SequencerData->GetFrameMax() : 0));


     // TODO: 이 시간에 맞춰 스켈레탈 메쉬의 포즈를 업데이트하는 로직 호출
     // 예: GEngine->GetSkeletalMeshViewer()->UpdateAnimationPose(PlaybackTime);


     SkeletalMeshComponent->SetAnimationTime(PlaybackTime); // USkeletalMeshComponent에 해당 함수 추가 필요
             // 또는 SkelComp->TickAnimation(DeltaTime, bLooping); 같은 함수 호출
         
     
 }


void AnimationSequenceViewerPanel::RenderNotifyControls()
{
    // "Add Notify" 버튼
    if (ImGui::Button("Add Notify"))
    {
        // 현재 재생 시간에 추가 요청
        RequestAddNotifyAtTime(PlaybackTime);
    }
    ImGui::SameLine(); // 같은 줄에 배치

    // 선택된 노티파이가 없으면 "Delete Notify" 버튼 비활성화
    bool canDelete = (SelectedNotifyIndex != -1);
    if (!canDelete) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f); // 반투명 처리
        ImGui::BeginDisabled(); // 버튼 비활성화 시작
    }
    // "Delete Notify" 버튼
    if (ImGui::Button("Delete Notify"))
    {
        if (canDelete) { // 삭제 가능한 상태일 때만 실행
            DeleteSelectedNotify();
        }
    }

    if (!canDelete) {
        ImGui::EndDisabled(); // 버튼 비활성화 종료
        ImGui::PopStyleVar(); // 스타일 복원
    }

    // 선택된 노티파이 정보 표시
    if (SelectedNotifyIndex != -1 && SelectedNotifyIndex < CurrentAnimSequence->GetAnimNotifies().Num())
    {
        ImGui::SameLine(); // 같은 줄에 배치
        // 선택된 노티파이 이름과 시간 표시
        
        float frameRate = CurrentAnimSequence->GetFrameRate().AsDecimal();
        int notifyFrame = static_cast<int>(CurrentAnimSequence->GetAnimNotifies()[SelectedNotifyIndex].TriggerTime * frameRate);
        
        ImGui::Text(" | Selected: %s @ %.3fs (Frame: %d)",
            (*CurrentAnimSequence->GetAnimNotifies()[SelectedNotifyIndex].NotifyName.ToString()),
            CurrentAnimSequence->GetAnimNotifies()[SelectedNotifyIndex].TriggerTime,
            notifyFrame);
    }
    
}

void AnimationSequenceViewerPanel::RenderAddNotifyPopup()
{
    // 팝업을 항상 화면 중앙에 표시
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // 모달 팝업 시작 ("Add New Notify" 창)
    if (ImGui::BeginPopupModal("Add New Notify", &bShowAddNotifyPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // 추가될 시간 표시
        int notifyFrame = static_cast<int>(AddNotifyTimeRequest * CurrentAnimSequence->GetFrameRate().AsDecimal());
        ImGui::Text("Add new notify at %.3f seconds (Frame: %d)", AddNotifyTimeRequest, notifyFrame);
        ImGui::Separator(); // 구분선

        // 노티파이 이름 입력 필드
        ImGui::InputText("Notify Name", AddNotifyNameBuffer, IM_ARRAYSIZE(AddNotifyNameBuffer));
        // TODO: 필요하다면 기존 노티파이 이름 드롭다운 또는 유효성 검사 추가

        // "OK" 버튼
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            if (strlen(AddNotifyNameBuffer) > 0) // 이름이 비어있지 않으면
            {
                // 새 노티파이 추가 함수 호출
                AddNewNotify(FName(AddNotifyNameBuffer), AddNotifyTimeRequest);
                bShowAddNotifyPopup = false; // 팝업 닫기 플래그 설정
                ImGui::CloseCurrentPopup(); // ImGui 팝업 닫기
            } else {
                // 선택 사항: 이름이 비어있을 경우 에러 메시지 표시
            }
        }
        ImGui::SetItemDefaultFocus(); // OK 버튼에 기본 포커스
        ImGui::SameLine(); // 같은 줄에 배치
        // "Cancel" 버튼
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            bShowAddNotifyPopup = false; // 팝업 닫기 플래그 설정
            ImGui::CloseCurrentPopup(); // ImGui 팝업 닫기
        }
        ImGui::EndPopup(); // 모달 팝업 종료
    }
}

void AnimationSequenceViewerPanel::ProcessNotifiesInRange(float StartTime, float EndTime, USkeletalMeshComponent* SkeletalMeshComponent)
{
     if (!CurrentAnimSequence) return;

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    AActor* TargetActor = nullptr; // 노티파이를 받을 액터 (예: 뷰어의 스켈레탈 메쉬 액터)
    TargetActor = SkeletalMeshComponent->GetOwner();
    if (!TargetActor) return;


    bool bForward = EndTime >= StartTime;

    for (const FAnimNotifyEvent& notify : CurrentAnimSequence->GetAnimNotifies())
    {
        bool bShouldTrigger = false;
        if (bForward) // 정방향
        {
            // (StartTime, EndTime] 구간에 TriggerTime이 포함되는지 확인
            if (notify.TriggerTime > StartTime && notify.TriggerTime <= EndTime)
            {
                bShouldTrigger = true;
            }
        }
        else // 역방향
        {
            // (EndTime, StartTime] 구간에 TriggerTime이 포함되는지 확인 (역방향이므로 EndTime < notify.TriggerTime <= StartTime)
            if (notify.TriggerTime > EndTime && notify.TriggerTime <= StartTime)
            {
                // 역재생 시 노티파이 트리거 여부는 정책에 따라 다름 (일반적으로는 안 함)
                // bShouldTrigger = true; // 필요하다면 활성화
            }
        }

        if (bShouldTrigger)
        {
            // 디버그 로그 또는 실제 이벤트 핸들링
            UE_LOG( ELogLevel::Display, TEXT("AnimNotify Triggered: %s at time %.2f"), *notify.NotifyName.ToString(), notify.TriggerTime);

            // 학습 내용의 HandleAnimNotify 와 유사한 방식으로 처리
            // 실제로는 UAnimInstance나 USkeletalMeshComponent를 통해 Owner Actor에게 전달
            // TargetActor->HandleAnimNotify(notify); // AActor에 해당 함수가 있다고 가정
            

            // SkelComp->HandleAnimNotify(notify); // USkeletalMeshComponent에 함수 추가
            if (AActor* OwnerActor = SkeletalMeshComponent->GetOwner())
            {
                // OwnerActor->HandleAnimNotify(notify); // AActor에 함수 추가
                // 또는 UAnimInstance를 통해 처리
                // if (UAnimInstance* AnimInst = SkelComp->GetAnimInstance()) // GetAnimInstance() 구현 필요
                // {
                //     // AnimInst->TriggerSingleAnimNotify(notify); // UAnimInstance에 함수 추가
                // }
            }
                
            
        }
    }
}


void AnimationSequenceViewerPanel::OnResize(HWND hWnd)
 {
     RECT ClientRect;
     GetClientRect(hWnd, &ClientRect);
     Width = ClientRect.right - ClientRect.left;
     Height = ClientRect.bottom - ClientRect.top;
 }
