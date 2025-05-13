#include "BoneHierarchyViewerPanel.h"

#include <iso646.h>
#include <ReferenceSkeleton.h>
#include "Engine/Classes/Engine/SkeletalMesh.h"
#include "Engine/Classes/Animation/Skeleton.h"
#include "Engine/Classes/Engine/FbxLoader.h"
BoneHierarchyViewerPanel::BoneHierarchyViewerPanel()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::SkeletalViewer);

}
void BoneHierarchyViewerPanel::Render()
{
    
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }

    if (BoneIconSRV == nullptr || NonWeightBoneIconSRV == nullptr) {
        LoadBoneIcon();
    }

    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.3f;

    float PanelPosX = (Width) * 0.8f+5.0f;
    float PanelPosY = 5.0f;

    ImVec2 MinSize(140, 100);
    ImVec2 MaxSize(FLT_MAX, 500);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;
    
    
    if (Engine->ActiveWorld) {
        if (Engine->ActiveWorld->WorldType == EWorldType::SkeletalViewer) {


            if (CopiedRefSkeleton == nullptr) {
                CopyRefSkeleton(); // 선택된 액터/컴포넌트로부터 스켈레톤 정보 복사
            }

            // CopiedRefSkeleton이 여전히 null이면 렌더링하지 않음
            if (CopiedRefSkeleton == nullptr || CopiedRefSkeleton->RawRefBoneInfo.IsEmpty()) {
                ImGui::Begin("Bone Hierarchy", nullptr, PanelFlags); // 창은 표시하되 내용은 비움
                ImGui::Text("No skeleton selected or skeleton has no bones.");
                ImGui::End();
                return;
            }

            ImGui::Begin("Bone Hierarchy", nullptr, PanelFlags); // 창 이름 변경

            // 검색 필터 추가 (선택 사항)
            // static char BoneSearchText[128] = "";
            // ImGui::InputText("Search", BoneSearchText, IM_ARRAYSIZE(BoneSearchText));
            // FString SearchFilter(BoneSearchText);

            // 루트 본부터 시작하여 트리 렌더링
            for (int32 i = 0; i < CopiedRefSkeleton->RawRefBoneInfo.Num(); ++i)
            {
                if (CopiedRefSkeleton->RawRefBoneInfo[i].ParentIndex == INDEX_NONE) // 루트 본인 경우
                {
                    // RenderBoneTree 호출 시 Engine 포인터 전달
                    RenderBoneTree(*CopiedRefSkeleton, i, Engine /*, SearchFilter */);
                }
            }
            ImGui::End();
        }

        float ExitPanelWidth = (Width) * 0.2f - 6.0f;
        float ExitPanelHeight = 30.0f;

        const float margin = 10.0f;

        float ExitPanelPosX = Width - ExitPanelWidth;
        float ExitPanelPosY = Height - ExitPanelHeight - 10;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::SetNextWindowSize(ImVec2(ExitPanelWidth, ExitPanelHeight), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(ExitPanelPosX, ExitPanelPosY), ImGuiCond_Always);
        
        constexpr ImGuiWindowFlags ExitPanelFlags =
            ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoBackground
            | ImGuiWindowFlags_NoScrollbar;

        ImGui::Begin("Exit Viewer", nullptr, ExitPanelFlags);
        if (ImGui::Button("Exit Viewer", ImVec2(ExitPanelWidth, ExitPanelHeight))) {
            UEditorEngine* EdEngine = Cast<UEditorEngine>(GEngine);
            EdEngine->EndSkeletalMeshViewer();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
 
}

void BoneHierarchyViewerPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void BoneHierarchyViewerPanel::SetSkeletalMesh(USkeletalMesh* SMesh)
{
    SkeletalMesh = SMesh;
}

int32 BoneHierarchyViewerPanel::GetSelectedBoneIndex() const
{
    return SelectedBoneIndex;
}

FString BoneHierarchyViewerPanel::GetSelectedBoneName() const
{
    if (SelectedBoneIndex == INDEX_NONE || !SkeletalMesh)
        return TEXT("");
    const auto& RefSkel = SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
    return RefSkel.RawRefBoneInfo[SelectedBoneIndex].Name.ToString();
}

void BoneHierarchyViewerPanel::LoadBoneIcon()
{
    BoneIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/Bone_16x.PNG")->TextureSRV;
    NonWeightBoneIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/BoneNonWeighted_16x.PNG")->TextureSRV;

}

void BoneHierarchyViewerPanel::CopyRefSkeleton()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    if (Engine->SkeletalMeshViewerWorld->GetSkeletalMeshComponent() == nullptr or
    Engine->SkeletalMeshViewerWorld->GetSkeletalMeshComponent()->GetSkeletalMeshAsset() == nullptr or
    Engine->SkeletalMeshViewerWorld->GetSkeletalMeshComponent()->GetSkeletalMeshAsset()->GetSkeleton() == nullptr) {
        return;
    }
    
    const FReferenceSkeleton& OrigRef = Engine->SkeletalMeshViewerWorld
        ->GetSkeletalMeshComponent()->GetSkeletalMeshAsset()
        ->GetSkeleton()->GetReferenceSkeleton();

    CopiedRefSkeleton = new FReferenceSkeleton();
    CopiedRefSkeleton->RawRefBoneInfo = OrigRef.RawRefBoneInfo;
    CopiedRefSkeleton->RawRefBonePose = OrigRef.RawRefBonePose;
    CopiedRefSkeleton->InverseBindPoseMatrices = OrigRef.InverseBindPoseMatrices;
    CopiedRefSkeleton->RawNameToIndexMap = OrigRef.RawNameToIndexMap;
}

void BoneHierarchyViewerPanel::RenderBoneTree(const FReferenceSkeleton& RefSkeleton, int32 BoneIndex, UEditorEngine* Engine /*, const FString& SearchFilter */)
{
    const FMeshBoneInfo& BoneInfo = CopiedRefSkeleton->RawRefBoneInfo[BoneIndex];
    const FString& ShortBoneName = GetCleanBoneName(BoneInfo.Name.ToString());

    // 검색 필터 적용 (선택 사항)
    // if (!SearchFilter.IsEmpty() && !ShortBoneName.Contains(SearchFilter))
    // {
    //    // 자식도 검색해야 하므로, 현재 노드가 필터에 맞지 않아도 자식은 재귀 호출
    //    bool bChildMatchesFilter = false;
    //    for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
    //    {
    //        if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
    //        {
    //            // 자식 중 하나라도 필터에 맞으면 현재 노드도 표시해야 할 수 있음 (복잡해짐)
    //            // 간단하게는 현재 노드가 안 맞으면 그냥 건너뛰도록 할 수 있음
    //        }
    //    }
    //    // 간단한 필터링: 현재 노드가 안 맞으면 그냥 숨김 (자식도 안 나옴)
    //    // if (!ShortBoneName.Contains(SearchFilter)) return;
    // }

    // 1) ImGui ID 충돌 방지
    ImGui::PushID(BoneIndex);

    ImGui::Image((ImTextureID)BoneIconSRV, ImVec2(16, 16));  // 16×16 픽셀 크기
    ImGui::SameLine();

    ImGuiTreeNodeFlags NodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
    // if (Engine->SkeletalMeshViewerWorld->SelectBoneIndex == BoneIndex) // 가상의 함수 호출
    // {
    //     NodeFlags |= ImGuiTreeNodeFlags_Selected; // 선택된 경우 Selected 플래그 추가
    // }

    // 자식이 없는 본은 리프 노드로 처리 (화살표 없음)
    bool bHasChildren = false;
    for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
    {
        if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
        {
            bHasChildren = true;
            break;
        }
    }
    if (!bHasChildren)
    {
        NodeFlags |= ImGuiTreeNodeFlags_Leaf; // 자식 없으면 리프 노드
        NodeFlags &= ~ImGuiTreeNodeFlags_OpenOnArrow; // 리프 노드는 화살표로 열 필요 없음
    }

    // ImGui::TreeNodeEx (본 이름, 플래그)
    // 이름 부분만 클릭 가능하도록 하려면 ImGui::Selectable을 함께 사용하거나 커스텀 로직 필요
    // 여기서는 TreeNodeEx 자체의 클릭 이벤트를 사용
    bool bNodeOpen = ImGui::TreeNodeEx(*ShortBoneName, NodeFlags);

    // --- 클릭 이벤트 처리 ---
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) // 왼쪽 마우스 버튼 클릭 시
    {
        // 엔진에 선택된 본 인덱스 설정 (가상의 함수 호출)
        Engine->SkeletalMeshViewerWorld->SelectBoneIndex = (BoneIndex);
    }

    if (bNodeOpen) // 노드가 열려있다면
    {
        // 자식 본들 재귀적으로 처리
        for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
        {
            if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
            {
                RenderBoneTree(RefSkeleton, i, Engine /*, SearchFilter */); // 재귀 호출 시 Engine 전달
            }
        }
        ImGui::TreePop(); // 트리 노드 닫기
    }

    ImGui::PopID(); // ID 스택 복원
}

FString BoneHierarchyViewerPanel::GetCleanBoneName(const FString& InFullName)
{
    // 1) 계층 구분자 '|' 뒤 이름만 취하기
    int32 barIdx = InFullName.FindChar(TEXT('|'),
        /*case*/ ESearchCase::CaseSensitive,
        /*dir*/  ESearchDir::FromEnd);
    FString name = (barIdx != INDEX_NONE)
        ? InFullName.RightChop(barIdx + 1)
        : InFullName;

    // 2) 네임스페이스 구분자 ':' 뒤 이름만 취하기
    int32 colonIdx = name.FindChar(TEXT(':'),
        /*case*/ ESearchCase::CaseSensitive,
        /*dir*/  ESearchDir::FromEnd);
    if (colonIdx != INDEX_NONE)
    {
        return name.RightChop(colonIdx + 1);
    }
    return name;
}
