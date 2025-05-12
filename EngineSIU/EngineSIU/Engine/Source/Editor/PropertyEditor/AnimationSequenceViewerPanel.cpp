#include "AnimationSequenceViewerPanel.h"
#include <chrono>
 #include "Engine/EditorEngine.h" // UEditorEngine 접근용
 #include "Engine/Classes/Animation/AnimSequence.h" // UAnimSequence 사용
#include "Engine/Source/Editor/UnrealEd/IconsFeather.h" // Feather Icons 헤더 (또는 사용 중인 아이콘 폰트 헤더)
#include "Font/IconDefs.h"


AnimationSequenceViewerPanel::AnimationSequenceViewerPanel()
 {
     // 이 패널이 어떤 월드 타입에서 활성화될지 설정 (예: SkeletalViewer 전용)
     SetSupportedWorldTypes(EWorldTypeBitFlag::Editor|EWorldTypeBitFlag::PIE|EWorldTypeBitFlag::SkeletalViewer);
     // 초기화 시 LastFrameTime 설정
     LastFrameTime = static_cast<float>(ImGui::GetTime());
 }

 AnimationSequenceViewerPanel::~AnimationSequenceViewerPanel()
 {
     delete SequencerData; // 동적 할당된 경우 해제
 }

 void AnimationSequenceViewerPanel::SetAnimationSequence(UAnimSequence* InAnimSequence)
 {
     CurrentAnimSequence = InAnimSequence;
     PlaybackTime = 0.0f;
     CurrentFrame = 0;
     FirstFrame = 0;
     bIsPlaying = false;

     delete SequencerData; // 기존 시퀀서 데이터 삭제
     if (CurrentAnimSequence)
     {
         SequencerData = new FSequenceInterface(CurrentAnimSequence);
     }
     else
     {
         SequencerData = nullptr;
     }
 }

 void AnimationSequenceViewerPanel::Render()
 {
     UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
     USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Engine->GetSelectedComponent());

     // 전 애니메이션과 달라지면 업데이트
     if (Engine and SkeletalMeshComponent)
     {
         static UAnimSequence* PrevAnim = nullptr;
         if (PrevAnim != SkeletalMeshComponent->GetAnimSequence())
         {
             SetAnimationSequence(SkeletalMeshComponent->GetAnimSequence());
         }
         PrevAnim = CurrentAnimSequence;
     }
     if (!Engine || !CurrentAnimSequence || !SequencerData || !SkeletalMeshComponent) // 엔진 또는 시퀀스 없으면 렌더링 안 함
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


    UpdatePlayback(deltaTime, SkeletalMeshComponent);
     

     // --- 패널 레이아웃 설정 (PropertyEditorPanel.cpp 참고) ---
     // 예시: 화면 하단에 길게 배치
     float PanelWidth = Width; // 전체 너비 사용
     float PanelHeight = Height * 0.25f; // 화면 높이의 25% 사용
     float PanelPosX = 0.0f;
     float PanelPosY = Height - PanelHeight; // 화면 하단

     ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);
     ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

     ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
     // ImGuiWindowFlags_MenuBar 플래그를 추가하여 메뉴바 공간 확보 가능

     ImGui::Begin("Animation Sequence Viewer", nullptr, PanelFlags);

     // 1. 재생 컨트롤 UI (버튼 등)
     RenderTimelineControls(SkeletalMeshComponent);
     ImGui::Separator();

     // 2. ImSequencer UI 호출
     // ImSequencer::Sequencer() 함수는 int* currentFrame을 받으므로, 멤버 변수 CurrentFrame을 사용합니다.
     // FirstFrame은 타임라인의 스크롤 위치를 제어합니다.
     // SelectedSequencerEntry는 시퀀서 내에서 아이템 선택 시 업데이트됩니다.
     int sequenceOptions = ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_CHANGE_FRAME; // 필요한 옵션 설정
     
     // CurrentFrame을 시퀀서에 전달하기 전에 PlaybackTime 기준으로 업데이트
     if (CurrentAnimSequence && CurrentAnimSequence->GetFrameRate().AsDecimal() > 0.0f)
     {
         CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
         // CurrentFrame이 시퀀스 범위를 벗어나지 않도록 클램핑
         CurrentFrame = std::max(SequencerData->GetFrameMin(), std::min(CurrentFrame, SequencerData->GetFrameMax()));
     }

     ImGui::PushID("MainSequencer");
     if (ImSequencer::Sequencer(SequencerData, &CurrentFrame, &bIsExpanded, &SelectedSequencerEntry, &FirstFrame, sequenceOptions))
     {
          //시퀀서에서 프레임이 변경된 경우 (예: 사용자가 재생 헤드를 드래그)
          if (CurrentAnimSequence && CurrentAnimSequence->GetFrameRate().AsDecimal() > 0.0f)
          {
              PlaybackTime = static_cast<float>(CurrentFrame) / CurrentAnimSequence->GetFrameRate().AsDecimal();
              // TODO: 이 시간에 맞춰 스켈레탈 메쉬의 포즈를 업데이트하는 로직 호출
              //Engine->SkeletalMeshViewerWorld->SetAnimationTime(PlaybackTime); 

              SkeletalMeshComponent->SetAnimationTime(PlaybackTime); // USkeletalMeshComponent에 해당 함수 추가 필요
              
          }
     }
     ImGui::PopID();

     static char NotifyName[256] = "";
     if (CurrentAnimSequence)
     {
         ImGui::InputText("NotifyName", NotifyName, sizeof(NotifyName));

         if (ImGui::Button("Add Notify"))
         {
             FAnimNotifyEvent NewNotify;
             NewNotify.NotifyName = FName(FString(NotifyName));
             NewNotify.TriggerTime = PlaybackTime;
             NewNotify.Duration = 0.0f;
             CurrentAnimSequence->AddAnimNotifyEvent(NewNotify);

             NotifyName[0] = '\0';
         }
     }

     if(CurrentAnimSequence->GetAnimNotifies().Num()>0)
         ImGui::Text("Edit Notify Trigger Time:");

     int NotifyIndex = 0;
     TArray<FAnimNotifyEvent> DeletedNotify;

     for(auto& Notify : CurrentAnimSequence->GetAnimNotifies())
     {
         ImGui::PushID(NotifyIndex);
         float FrameRate = CurrentAnimSequence->GetFrameRate().AsDecimal();
         float TimeInSeconds = Notify.TriggerTime;

         ImGui::Separator();
         ImGui::Text(*Notify.NotifyName.ToString());
         ImGui::SameLine();
         if (ImGui::SliderFloat("##TriggerTime", &TimeInSeconds, 0.0f, CurrentAnimSequence->GetPlayLength(), "%.2f s"))
         {
             Notify.TriggerTime = TimeInSeconds;
         }
         ImGui::SameLine();
         if (ImGui::Button("Delete"))
         {
             DeletedNotify.Add(Notify);
         }
         ImGui::PopID();
         ++NotifyIndex;
     }

     for (auto& Notify : DeletedNotify)
     {
         CurrentAnimSequence->RemoveAnimNotifyEvent(Notify);
     }

     // 선택된 트랙 정보 표시 (예시)
     if (SelectedSequencerEntry != -1)
     {
         ImGui::Text("Selected Track: %s", SequencerData->GetItemLabel(SelectedSequencerEntry));
     }

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


     // 현재 프레임 업데이트 (ImSequencer는 CurrentFrame을 직접 사용)
     CurrentFrame = static_cast<int>(PlaybackTime * frameRate);
     CurrentFrame = std::max(0, std::min(CurrentFrame, SequencerData ? SequencerData->GetFrameMax() : 0));


     // TODO: 이 시간에 맞춰 스켈레탈 메쉬의 포즈를 업데이트하는 로직 호출
     // 예: GEngine->GetSkeletalMeshViewer()->UpdateAnimationPose(PlaybackTime);


     SkeletalMeshComponent->SetAnimationTime(PlaybackTime); // USkeletalMeshComponent에 해당 함수 추가 필요
             // 또는 SkelComp->TickAnimation(DeltaTime, bLooping); 같은 함수 호출
         
     
 }


 void AnimationSequenceViewerPanel::OnResize(HWND hWnd)
 {
     RECT ClientRect;
     GetClientRect(hWnd, &ClientRect);
     Width = ClientRect.right - ClientRect.left;
     Height = ClientRect.bottom - ClientRect.top;
 }
