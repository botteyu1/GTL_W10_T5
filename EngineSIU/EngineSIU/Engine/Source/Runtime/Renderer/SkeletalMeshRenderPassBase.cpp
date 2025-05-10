#include "SkeletalMeshRenderPassBase.h"

#include "Animation/Skeleton.h"
#include "UObject/UObjectIterator.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"
#include "Editor/PropertyEditor/ShowFlags.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Engine/AssetManager.h"
#include "RendererHelpers.h"

class UEditorEngine;

FSkeletalMeshRenderPassBase::FSkeletalMeshRenderPassBase()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FSkeletalMeshRenderPassBase::~FSkeletalMeshRenderPassBase()
{
    ReleaseResource();
}

void FSkeletalMeshRenderPassBase::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;

    CreateResource();
}

void FSkeletalMeshRenderPassBase::PrepareRenderArr()
{
    for (const auto iter : TObjectRange<USkeletalMeshComponent>())
    {
        if (iter->GetWorld() != GEngine->ActiveWorld)
        {
            continue;
        }
        SkeletalMeshComponents.Add(iter);
    }
}

void FSkeletalMeshRenderPassBase::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRenderPass(Viewport);

    Render_Internal(Viewport);

    CleanUpRenderPass(Viewport);
}

void FSkeletalMeshRenderPassBase::ClearRenderArr()
{
    SkeletalMeshComponents.Empty();
}

void FSkeletalMeshRenderPassBase::CreateResource()
{
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof(FMatrix) * MaxBoneNum;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.StructureByteStride = sizeof(FMatrix);
    
    // 6. 버퍼 생성 및 쉐이더 리소스 뷰 생성
    ID3D11Buffer* boneBuffer = nullptr;
    ID3D11ShaderResourceView* boneBufferSRV = nullptr;
    
    // BufferManager를 통해 버퍼 생성 요청
    HRESULT hr = Graphics->Device->CreateBuffer(&BufferDesc, nullptr, &BoneBuffer);
    if (FAILED(hr))
    {
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SrvDesc.Buffer.FirstElement = 0;
    SrvDesc.Buffer.NumElements = MaxBoneNum;
    
    hr = Graphics->Device->CreateShaderResourceView(BoneBuffer, nullptr, &BoneSRV);
    if (FAILED(hr))
    {
        return;
    }
}

void FSkeletalMeshRenderPassBase::ReleaseResource()
{
    if (BoneSRV)
    {
        BoneSRV->Release();
        BoneSRV = nullptr;
    }

    if (BoneBuffer)
    {
        BoneBuffer->Release();
        BoneBuffer = nullptr;
    }
}

void FSkeletalMeshRenderPassBase::Render_Internal(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    RenderAllSkeletalMeshes(Viewport);
}

void FSkeletalMeshRenderPassBase::RenderAllSkeletalMeshes(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    for (USkeletalMeshComponent* Comp : SkeletalMeshComponents)
    {
        if (!Comp || !Comp->GetSkeletalMeshAsset())
        {
            continue;
        }

        const FSkeletalMeshRenderData* RenderData = Comp->GetSkeletalMeshAsset()->GetRenderData();
        if (RenderData == nullptr)
        {
            continue;
        }

        UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

        FMatrix WorldMatrix = Comp->GetWorldMatrix();
        FVector4 UUIDColor = Comp->EncodeUUID() / 255.0f;
        const bool bIsSelected = (Engine && Engine->GetSelectedActor() == Comp->GetOwner());

        UpdateObjectConstant(WorldMatrix, UUIDColor, bIsSelected);

        if (FRenderingSettings::bIsCPUSkinning == false)
        {
            UpdateBone(Comp);
            
            RenderSkeletalMesh(RenderData);
        }
        else
        {
            RenderCPUSkinningSkeletalMesh(Comp);
        }


        if (Viewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            FEngineLoop::PrimitiveDrawBatch.AddAABBToBatch(Comp->GetBoundingBox(), Comp->GetComponentLocation(), WorldMatrix);
        }
    }
}

void FSkeletalMeshRenderPassBase::RenderSkeletalMesh(const FSkeletalMeshRenderData* RenderData) const
{
    UINT Stride = sizeof(FSkeletalMeshVertex);
    UINT Offset = 0;

    FVertexInfo VertexInfo;
    BufferManager->CreateVertexBuffer(RenderData->ObjectName, RenderData->Vertices, VertexInfo);

    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexInfo.VertexBuffer, &Stride, &Offset);

    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer(RenderData->ObjectName, RenderData->Indices, IndexInfo);
    if (IndexInfo.IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(IndexInfo.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }
    else
    {
        Graphics->DeviceContext->Draw(RenderData->Vertices.Num(), 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        FName MaterialName = RenderData->MaterialSubsets[SubMeshIndex].MaterialName;
        UMaterial* Material = UAssetManager::Get().GetMaterial(MaterialName);
        FMaterialInfo materilinfo = Material->GetMaterialInfo();
        MaterialUtils::UpdateMaterial(BufferManager, Graphics, materilinfo);

        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount; 
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FSkeletalMeshRenderPassBase::RenderCPUSkinningSkeletalMesh(const USkeletalMeshComponent* SkeletalMeshComponent) const
{
    const FSkeletalMeshRenderData* RenderData = SkeletalMeshComponent->GetSkeletalMeshAsset()->GetRenderData();

    TArray<FSkeletalMeshVertex> SkinnedVertices;
    ComputeCPUSkinningForMesh(SkeletalMeshComponent, SkinnedVertices);
    
    UINT Stride = sizeof(FSkeletalMeshVertex);
    UINT Offset = 0;

    FVertexInfo VertexInfo;
    BufferManager->CreateDynamicVertexBuffer(RenderData->ObjectName, SkinnedVertices, VertexInfo);

    BufferManager->UpdateDynamicVertexBuffer(RenderData->ObjectName, SkinnedVertices);
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexInfo.VertexBuffer, &Stride, &Offset);

    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer(RenderData->ObjectName, RenderData->Indices, IndexInfo);
    if (IndexInfo.IndexBuffer)
    {
        Graphics->DeviceContext->IASetIndexBuffer(IndexInfo.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }
    else
    {
        Graphics->DeviceContext->Draw(RenderData->Vertices.Num(), 0);
        return;
    }

    for (int SubMeshIndex = 0; SubMeshIndex < RenderData->MaterialSubsets.Num(); SubMeshIndex++)
    {
        FName MaterialName = RenderData->MaterialSubsets[SubMeshIndex].MaterialName;
        UMaterial* Material = UAssetManager::Get().GetMaterial(MaterialName);
        FMaterialInfo materilinfo = Material->GetMaterialInfo();
        MaterialUtils::UpdateMaterial(BufferManager, Graphics, materilinfo);

        uint32 StartIndex = RenderData->MaterialSubsets[SubMeshIndex].IndexStart;
        uint32 IndexCount = RenderData->MaterialSubsets[SubMeshIndex].IndexCount; 
        Graphics->DeviceContext->DrawIndexed(IndexCount, StartIndex, 0);
    }
}

void FSkeletalMeshRenderPassBase::RenderSkeletalMesh(ID3D11Buffer* Buffer, UINT VerticesNum) const
{
}

void FSkeletalMeshRenderPassBase::RenderSkeletalMesh(ID3D11Buffer* VertexBuffer, ID3D11Buffer* IndexBuffer, UINT IndicesNum) const
{
}

void FSkeletalMeshRenderPassBase::UpdateObjectConstant(const FMatrix& WorldMatrix, const FVector4& UUIDColor, bool bIsSelected) const
{
    FObjectConstantBuffer ObjectData = {};
    ObjectData.WorldMatrix = WorldMatrix;
    ObjectData.InverseTransposedWorld = FMatrix::Transpose(FMatrix::Inverse(WorldMatrix));
    ObjectData.UUIDColor = UUIDColor;
    ObjectData.bIsSelected = bIsSelected;
    ObjectData.bIsCPUSkinning = FRenderingSettings::bIsCPUSkinning;

    BufferManager->UpdateConstantBuffer(TEXT("FObjectConstantBuffer"), ObjectData);
}

void FSkeletalMeshRenderPassBase::UpdateBone(const USkeletalMeshComponent* SkeletalMeshComponent)
{
    if (!SkeletalMeshComponent ||
        !SkeletalMeshComponent->GetSkeletalMeshAsset() ||
        !SkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton())
    {
        return;
    }

    TArray<FMatrix> SkinningBoneMatrices;

    ComputeSkinningMatrices(SkeletalMeshComponent, SkinningBoneMatrices);
    

    // 스트럭처드 버퍼는 row_major와 같은 레이아웃 지정자를 사용할 수 없기에 트랜스포즈해서 전달
    for (int32 BoneIndex = 0; BoneIndex < SkinningBoneMatrices.Num(); ++BoneIndex)
    {
        SkinningBoneMatrices[BoneIndex] = FMatrix::Transpose(SkinningBoneMatrices[BoneIndex]);
    }
    // Update
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(BoneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Buffer Map 실패, HRESULT: 0x%X"), hr);
        return;
    }
    
    ZeroMemory(MappedResource.pData, sizeof(FMatrix) * MaxBoneNum);
    memcpy(MappedResource.pData, SkinningBoneMatrices.GetData(), sizeof(FMatrix) * SkinningBoneMatrices.Num());
    Graphics->DeviceContext->Unmap(BoneBuffer, 0); 
}


void FSkeletalMeshRenderPassBase::ComputeSkinningMatrices(const USkeletalMeshComponent* SkeletalMeshComponent,
    TArray<FMatrix>& OutSkinningMatrices) const
{
    if (!SkeletalMeshComponent ||
        !SkeletalMeshComponent->GetSkeletalMeshAsset() ||
        !SkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton())
    {
        return;
    }

    // Skeleton 정보 가져오기
    const USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();
    const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
    const int32 BoneNum = RefSkeleton.RawRefBoneInfo.Num();

    // 현재 애니메이션 본 행렬 계산
    TArray<FMatrix> CurrentGlobalBoneMatrices;
    SkeletalMeshComponent->GetCurrentGlobalBoneMatrices(CurrentGlobalBoneMatrices);
    
    // 최종 스키닝 행렬 계산
    OutSkinningMatrices.SetNum(BoneNum);
    
    for (int32 BoneIndex = 0; BoneIndex < BoneNum; ++BoneIndex)
    {
        OutSkinningMatrices[BoneIndex] = RefSkeleton.InverseBindPoseMatrices[BoneIndex] * CurrentGlobalBoneMatrices[BoneIndex];
       // OutSkinningMatrices[BoneIndex] = FMatrix::Transpose(OutSkinningMatrices[BoneIndex]);
    }
}

void FSkeletalMeshRenderPassBase::ComputeCPUSkinningForMesh(const USkeletalMeshComponent* SkeletalMeshComponent,
    TArray<FSkeletalMeshVertex>& OutVertices) const
{

    const FSkeletalMeshRenderData* RenderData = SkeletalMeshComponent->GetSkeletalMeshAsset()->GetRenderData();

    const TArray<FSkeletalMeshVertex>& InVertices = RenderData->Vertices;

    TArray<FMatrix> SkinningBoneMatrices;

    ComputeSkinningMatrices(SkeletalMeshComponent, SkinningBoneMatrices);
    
    OutVertices.SetNum(InVertices.Num()); // 결과 배열 크기 설정
    
    for (int32 VertIndex = 0; VertIndex < InVertices.Num(); ++VertIndex)
    {
        OutVertices[VertIndex] = ComputeCPUSkinningForVertex(
            InVertices[VertIndex],SkinningBoneMatrices
        );
    }

}

FSkeletalMeshVertex FSkeletalMeshRenderPassBase::ComputeCPUSkinningForVertex(const FSkeletalMeshVertex& InVertex,
                                                                             const TArray<FMatrix>& InSkinningBoneMatrices) const
{
    const FSkeletalMeshVertex& RefVertex = InVertex;
    FSkeletalMeshVertex OutVertex;
    const FVector RefPosition = FVector(RefVertex.X, RefVertex.Y, RefVertex.Z); // 모델 공간 위치
    const FVector RefNormal = FVector(RefVertex.NormalX, RefVertex.NormalY, RefVertex.NormalZ); // 모델 공간 노멀

    // --- 1. 스키닝 처리 (오브젝트 공간) ---
    FVector SkinnedPosition(0.f, 0.f, 0.f); // Object Space
    FVector skinnedNormalOS(0.f, 0.f, 0.f);
    FVector skinnedTangentOS(0.f, 0.f, 0.f); // 탄젠트도 스키닝

    float totalWeight = 0.0f;

    for (int i = 0; i < 4; ++i)
    {
        float Weight = InVertex.BoneWeights[i];
        totalWeight += Weight;

        if (Weight > 0.00001f) // 부동소수점 비교 시 오차 감안
        {
            uint32_t boneIdx = InVertex.BoneIndices[i];
            if (boneIdx >= InSkinningBoneMatrices.Num()) { // 본 인덱스 범위 체크
                // UE_LOG(LogTemp, Error, TEXT("Invalid bone index: %u"), boneIdx);
                continue;
            }
            const FMatrix& SkinningboneTransform = InSkinningBoneMatrices[boneIdx];

            // 위치 스키닝 (w=1로 변환)
            SkinnedPosition += SkinningboneTransform.TransformPosition(RefPosition) * Weight;
            
            // 법선 스키닝 (w=0으로 변환, 방향 벡터이므로 이동(translation) 무시)
            skinnedNormalOS += SkinningboneTransform.TransformDirection(RefNormal) * Weight;

            // 탄젠트 스키닝 (w=0으로 변환)
            FVector tangentXYZ = FVector(InVertex.Tangent.X, InVertex.Tangent.Y, InVertex.Tangent.Z);
            FVector transformedTangentPart = SkinningboneTransform.TransformDirection(tangentXYZ);
            skinnedTangentOS.X += Weight * transformedTangentPart.X;
            skinnedTangentOS.Y += Weight * transformedTangentPart.Y;
            skinnedTangentOS.Z += Weight * transformedTangentPart.Z;
        }
    }

    // 가중치 예외 처리 및 정규화
    if (totalWeight < 0.001f)
    {
        SkinnedPosition = FVector{InVertex.Position.X, InVertex.Position.Y, InVertex.Position.Z};
        skinnedNormalOS = InVertex.Normal;
        skinnedTangentOS = FVector{InVertex.Tangent.X, InVertex.Tangent.Y, InVertex.Tangent.Z};
    }
    else if (abs(totalWeight - 1.0f) > 0.001f)
    {
        SkinnedPosition.X /= totalWeight;
        SkinnedPosition.Y /= totalWeight;
        SkinnedPosition.Z /= totalWeight;
        // 일반적으로 SkinnedPosition.W는 1이 되도록 유지하는 것이 좋으나, HLSL 코드를 따름

        skinnedNormalOS.X /= totalWeight;
        skinnedNormalOS.Y /= totalWeight;
        skinnedNormalOS.Z /= totalWeight;

        skinnedTangentOS.X /= totalWeight;
        skinnedTangentOS.Y /= totalWeight;
        skinnedTangentOS.Z /= totalWeight;
    }

    skinnedNormalOS.Normalize();
    skinnedTangentOS.Normalize();
    
    if (!skinnedNormalOS.IsNearlyZero() && !skinnedTangentOS.IsNearlyZero()) // 0벡터 방지
    {
        skinnedTangentOS = skinnedTangentOS - skinnedNormalOS * FVector::DotProduct(skinnedNormalOS, skinnedTangentOS);
    }
    skinnedTangentOS.Normalize();

    OutVertex.Position = SkinnedPosition;
    OutVertex.Normal = skinnedNormalOS;
    OutVertex.Tangent = skinnedTangentOS;
    OutVertex.TangentW = InVertex.Tangent.W;

    return OutVertex;
}

