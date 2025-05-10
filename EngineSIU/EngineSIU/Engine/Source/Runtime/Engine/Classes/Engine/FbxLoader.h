#pragma once

#include <fbxsdk.h>

#include "StaticMesh.h"
#include "HAL/PlatformType.h"
#include "Container/Array.h"
#include "Container/Map.h"

struct FSkeletalMeshVertex;
struct FMaterialInfo;
struct FReferenceSkeleton;
struct FTransform;
struct FMeshBoneInfo;
class USkeleton;
class FString;
class USkeletalMesh;
struct FSkeletalMeshRenderData;
struct FFbxLoadResult;
struct FMatrix;
struct FAnimationCurveData;
class UAnimDataModel;
struct FBoneAnimationTrack;
struct FFrameRate;

class FFbxLoader
{
public:
    FFbxLoader();
    ~FFbxLoader();

    FFbxLoadResult LoadFBX(const FString& InFilePath);

private:
    FbxManager* Manager;
    FbxImporter* Importer;
    FbxScene* Scene;

    FWString FilePath;
    FWString ObjectName;
    FString DisplayName;

    // Begin Material
    void ProcessMaterials(FFbxLoadResult& OutResult);

    FMaterialInfo ExtractMaterialsFromFbx(FbxSurfaceMaterial* FbxMaterial);

    void ExtractTextureInfoFromFbx(FbxSurfaceMaterial* FbxMaterial, FMaterialInfo& OutMaterialInfo);
    // End Material

    // Begin Skeleton
    void ProcessSkeletonHierarchy(FbxNode* RootNode, FFbxLoadResult& OutResult);

    FbxPose* FindBindPose(FbxNode* SkeletonRoot);

    void CollectSkeletonBoneNodes(FbxNode* Node, TArray<FbxNode*>& OutBoneNodes);
    
    void FindSkeletonRootNodes(FbxNode* Node, TArray<FbxNode*>& OutSkeletonRoots);

    bool IsSkeletonRootNode(FbxNode* Node);

    void BuildSkeletonHierarchy(FbxNode* SkeletonRoot, USkeleton* OutSkeleton, FbxPose* BindPose);

    void CollectBoneData(FbxNode* Node, FReferenceSkeleton& OutReferenceSkeleton, int32 ParentIndex, FbxPose* BindPose);

    FTransform ConvertFbxTransformToFTransform(FbxNode* Node, FbxTime Time = FBXSDK_TIME_INFINITE) const;
    // End Skeleton
    
    // Begin Mesh
    void ProcessMeshes(FbxNode* Node, FFbxLoadResult& OutResult);

    void CollectMeshNodes(FbxNode* Node, const TArray<USkeleton*>& Skeletons, TMap<USkeleton*, TArray<FbxNode*>>& OutSkeletalMeshNodes , TArray<FbxNode*>& OutStaticMeshNodes);

    USkeletalMesh* CreateSkeletalMeshFromNodes(const TArray<FbxNode*>& MeshNodes, USkeleton* Skeleton, int32 GlobalMeshIdx);

    UStaticMesh* CreateStaticMesh(FbxNode* MeshNode, int32 GlobalMeshIdx);

    template<typename T>
    void CalculateTangents(TArray<T>& Vertices, const TArray<uint32>& Indices);

    template<typename T>
    void CalculateTangent_Internal(T& PivotVertex, const T& Vertex1, const T& Vertex2);
    
    USkeleton* FindAssociatedSkeleton(FbxNode* Node, const TArray<USkeleton*>& Skeletons);

    void ExtractBindPoseMatrices(const FbxMesh* Mesh, const USkeleton* Skeleton, TArray<FMatrix>& OutInverseBindPoseMatrices) const;
    
    FMatrix ConvertFbxMatrixToFMatrix(const FbxAMatrix& FbxMatrix) const;
    // End Mesh

    // Begin Animation
    void ProcessAnimation(FbxNode* Node, FFbxLoadResult& OutResult);

    void CollectAnimationStacks(FbxScene* Scene, TArray<FbxAnimStack*>& OutAnimationStacks);

    UAnimDataModel* CreateAnimDataModelFromFbxAnimStack(FbxAnimStack* AnimStack, FbxTime::EMode TimeNode, const TArray<FbxNode*>& BoneNodes);

    FAnimationCurveData ExtractAnimationCurveData(FbxAnimStack* AnimStack);

    void ExtractBoneAnimationTracks(FbxAnimStack* AnimStack, const TArray<FbxNode*> BoneNodes, UAnimDataModel* AnimDataModel);
    
    void AddCurveDataFromFbx(FbxAnimCurve* FbxCurve, const FName& curveFName, FAnimationCurveData& outAnimationCurveData);

    FFrameRate GetFrameRateFromFbxTimeMode(FbxTime::EMode TimeMode);

    // 좌표계 변환 메소드
    void ConvertSceneToLeftHandedZUpXForward(FbxScene* Scene);

    bool CreateTextureFromFile(const FWString& Filename, bool bIsSRGB);
};

template <typename T>
void FFbxLoader::CalculateTangents(TArray<T>& Vertices, const TArray<uint32>& Indices)
{
    // 탄젠트 초기화
    for (T& Vertex : Vertices)
    {
        Vertex.TangentX = 0.0f;
        Vertex.TangentY = 0.0f;
        Vertex.TangentZ = 0.0f;
        Vertex.TangentW = 0.0f;
    }

    // 각 삼각형마다 탄젠트 계산
    for (int32 i = 0; i < Indices.Num(); i += 3)
    {
        T& V0 = Vertices[static_cast<int32>(Indices[i])];
        T& V1 = Vertices[static_cast<int32>(Indices[i + 1])];
        T& V2 = Vertices[static_cast<int32>(Indices[i + 2])];
        
        CalculateTangent_Internal(V0, V1, V2);
        CalculateTangent_Internal(V1, V2, V0);
        CalculateTangent_Internal(V2, V0, V1);
    }

    // 각 정점의 탄젠트 정규화
    for (T& Vertex : Vertices)
    {
        FVector Tangent(Vertex.TangentX, Vertex.TangentY, Vertex.TangentZ);
        if (!Tangent.IsNearlyZero())
        {
            Tangent.Normalize();
        }
        else
        {
            // 탄젠트를 계산할 수 없는 경우 기본값 설정
            FVector Normal(Vertex.NormalX, Vertex.NormalY, Vertex.NormalZ);
            FVector Arbitrary = FMath::Abs(Normal.Z) < 0.99f ? FVector(0, 0, 1) : FVector(1, 0, 0);
            Tangent = FVector::CrossProduct(Normal, Arbitrary).GetSafeNormal();
        }
        
        Vertex.TangentX = Tangent.X;
        Vertex.TangentY = Tangent.Y;
        Vertex.TangentZ = Tangent.Z;
    }
}

template <typename T>
void FFbxLoader::CalculateTangent_Internal(T& PivotVertex, const T& Vertex1, const T& Vertex2)
{
    const float s1 = Vertex1.U - PivotVertex.U;
    const float t1 = Vertex1.V - PivotVertex.V;
    const float s2 = Vertex2.U - PivotVertex.U;
    const float t2 = Vertex2.V - PivotVertex.V;
    const float E1x = Vertex1.X - PivotVertex.X;
    const float E1y = Vertex1.Y - PivotVertex.Y;
    const float E1z = Vertex1.Z - PivotVertex.Z;
    const float E2x = Vertex2.X - PivotVertex.X;
    const float E2y = Vertex2.Y - PivotVertex.Y;
    const float E2z = Vertex2.Z - PivotVertex.Z;

    const float Denominator = s1 * t2 - s2 * t1;
    FVector Tangent(1, 0, 0);
    FVector BiTangent(0, 1, 0);
    FVector Normal(PivotVertex.NormalX, PivotVertex.NormalY, PivotVertex.NormalZ);
    
    if (FMath::Abs(Denominator) > SMALL_NUMBER)
    {
        // 정상적인 계산 진행
        const float f = 1.f / Denominator;
        
        const float Tx = f * (t2 * E1x - t1 * E2x);
        const float Ty = f * (t2 * E1y - t1 * E2y);
        const float Tz = f * (t2 * E1z - t1 * E2z);
        Tangent = FVector(Tx, Ty, Tz).GetSafeNormal();

        const float Bx = f * (-s2 * E1x + s1 * E2x);
        const float By = f * (-s2 * E1y + s1 * E2y);
        const float Bz = f * (-s2 * E1z + s1 * E2z);
        BiTangent = FVector(Bx, By, Bz).GetSafeNormal();
    }
    else
    {
        // 대체 탄젠트 계산 방법
        // 방법 1: 다른 방향에서 탄젠트 계산 시도
        FVector Edge1(E1x, E1y, E1z);
        FVector Edge2(E2x, E2y, E2z);
    
        // 기하학적 접근: 두 에지 사이의 각도 이등분선 사용
        Tangent = (Edge1.GetSafeNormal() + Edge2.GetSafeNormal()).GetSafeNormal();
    
        // 만약 두 에지가 평행하거나 반대 방향이면 다른 방법 사용
        if (Tangent.IsNearlyZero())
        {
            // TODO: 기본 축 방향 중 하나 선택 (메시의 주 방향에 따라 선택)
            Tangent = FVector(1.0f, 0.0f, 0.0f);
        }
    }

    Tangent = (Tangent - Normal * FVector::DotProduct(Normal, Tangent)).GetSafeNormal();
    
    const float Sign = (FVector::DotProduct(FVector::CrossProduct(Normal, Tangent), BiTangent) < 0.f) ? -1.f : 1.f;

    PivotVertex.TangentX = Tangent.X;
    PivotVertex.TangentY = Tangent.Y;
    PivotVertex.TangentZ = Tangent.Z;
    PivotVertex.TangentW = Sign;
}
