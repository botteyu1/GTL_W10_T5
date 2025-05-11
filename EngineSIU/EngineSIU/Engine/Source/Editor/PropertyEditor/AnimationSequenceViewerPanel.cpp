#include "AnimationSequenceViewerPanel.h"
#include <chrono>
 #include "Engine/EditorEngine.h" // UEditorEngine 접근용
 #include "Engine/Classes/Animation/AnimSequence.h" // UAnimSequence 사용



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
         SequencerData = new MySequence(CurrentAnimSequence);
     }
     else
     {
         SequencerData = nullptr;
         SequencerData = new MySequence(); // 기본값으로 초기화
     }
 }

 void AnimationSequenceViewerPanel::Render()
 {
     UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
     USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Engine->GetSelectedComponent());

     //TODO 테스트 코드
     if (Engine and SkeletalMeshComponent)
     {
         static UAnimSequence* PrevAnim = nullptr;
         if (PrevAnim != SkeletalMeshComponent->AnimSequence)
         {
             SetAnimationSequence(SkeletalMeshComponent->AnimSequence);
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

     if (bIsPlaying)
     {
         UpdatePlayback(deltaTime, SkeletalMeshComponent);
     }

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
     
     // 선택된 트랙 정보 표시 (예시)
     if (SelectedSequencerEntry != -1)
     {
         ImGui::Text("Selected Track: %s", SequencerData->GetItemLabel(SelectedSequencerEntry));
     }

     ImGui::End();
 }

 void AnimationSequenceViewerPanel::RenderTimelineControls(USkeletalMeshComponent* SkeletalMeshComp)
 {
     // 재생/정지 버튼
     if (bIsPlaying)
     {
         if (ImGui::Button("Stop")) // ICON_FA_STOP
         {
             bIsPlaying = false;
         }
     }
     else
     {
         if (ImGui::Button("Play")) // ICON_FA_PLAY
         {
             bIsPlaying = true;
             // 재생 시작 시 LastFrameTime을 현재 시간으로 리셋하여 델타 타임 점프 방지
             LastFrameTime = static_cast<float>(ImGui::GetTime());
         }
     }
     ImGui::SameLine();

     // 처음으로 이동 버튼
     if (ImGui::Button("|<")) // ICON_FA_FAST_BACKWARD
     {
         PlaybackTime = 0.0f;
         CurrentFrame = 0;
         bIsPlaying = false; // 처음으로 가면 일단 정지


         SkeletalMeshComp->SetAnimationTime(PlaybackTime);
         
     }
     ImGui::SameLine();

     // 끝으로 이동 버튼
     if (ImGui::Button(">|")) // ICON_FA_FAST_FORWARD
     {
         if (CurrentAnimSequence)
         {
             PlaybackTime = CurrentAnimSequence->GetPlayLength();
             CurrentFrame = SequencerData ? SequencerData->GetFrameMax() : 0;
             bIsPlaying = false; // 끝으로 가면 일단 정지
             // TODO: 스켈레탈 메쉬 포즈 업데이트
             SkeletalMeshComp->SetAnimationTime(PlaybackTime);
         }
     }
     ImGui::SameLine();

     // 현재 시간 / 전체 시간 표시
     if (CurrentAnimSequence)
     {
            ImGui::Text("%.2f / %.2f s", PlaybackTime, CurrentAnimSequence->GetPlayLength());
         ImGui::SameLine();
         ImGui::Text("Frame: %d / %d", CurrentFrame, SequencerData ? SequencerData->GetFrameMax() : 0);
     }

     // TODO: 슬라이더로 시간 직접 조절 (ImGui::SliderFloat)
     ImGui::PushItemWidth(-1); // 슬라이더가 남은 공간을 꽉 채우도록
     if (CurrentAnimSequence && CurrentAnimSequence->GetPlayLength() > 0)
     {
         if (ImGui::SliderFloat("##Timeline", &PlaybackTime, 0.0f, CurrentAnimSequence->GetPlayLength(), "%.2f s"))
         {
             bIsPlaying = false; // 슬라이더 조작 시 자동 재생 정지 (선택 사항)
             CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
             // TODO: 스켈레탈 메쉬 포즈 업데이트
         }
     }
     ImGui::PopItemWidth();
 }

 void AnimationSequenceViewerPanel::UpdatePlayback(float DeltaTime, USkeletalMeshComponent* SkeletalMeshComponent)
 {
     if (!CurrentAnimSequence || !bIsPlaying)
     {
         return;
     }

     PlaybackTime += DeltaTime;
     float sequenceLength = CurrentAnimSequence->GetPlayLength();

     if (PlaybackTime >= sequenceLength)
     {
         // TODO: 루프 옵션에 따라 처리
         //PlaybackTime = sequenceLength; // 또는 0.0f로 리셋 (루프 시)
         PlaybackTime = 0.0f;
         bIsPlaying = false; // 일단 한 번 재생 후 정지 (루프 미구현 시)
     }

     // 현재 프레임 업데이트 (ImSequencer는 CurrentFrame을 직접 사용)
     CurrentFrame = static_cast<int>(PlaybackTime * CurrentAnimSequence->GetFrameRate().AsDecimal());
     CurrentFrame = 0.0f;
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
