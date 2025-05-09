#pragma once

#include "Define.h"
#include "Hal/PlatformType.h"
#include "Container/Array.h"

struct FSkeletalMeshVertex
{
    union
    {
        FVector Position; 
        struct
        {
            float X, Y, Z;     
        };
    };

    union
    {
        FLinearColor Color; 
        struct
        {
            float R, G, B, A;   
        };
    };

    union
    {
        FVector Normal;   
        struct
        {
            float NormalX, NormalY, NormalZ;
        };
    };

    union
    {
        FVector4 Tangent; 
        struct
        {
            float TangentX, TangentY, TangentZ, TangentW;
        };
    };

    union
    {
        FVector2D UVs;          
        struct
        {
            float U, V;
        };
    };

    uint32 BoneIndices[4];
    float BoneWeights[4];

    FSkeletalMeshVertex()
        : Position(FVector::ZeroVector) 
        , Color(0.5f, 0.5f, 0.5f, 0.5f)  
        , Normal(FVector::ZeroVector)  
        , Tangent(FVector4(0.f, 0.f, 0.f, 0.f)) 
        , UVs(FVector2D::ZeroVector)          
    {
        BoneIndices[0] = BoneIndices[1] = BoneIndices[2] = BoneIndices[3] = 0;
        BoneWeights[0] = BoneWeights[1] = BoneWeights[2] = BoneWeights[3] = 0.f;
    }
};

struct FSkeletalMeshRenderData
{
    FWString ObjectName;
    FString DisplayName;

    TArray<FSkeletalMeshVertex> Vertices;
    TArray<UINT> Indices;

    TArray<FMaterialInfo> Materials;
    TArray<FMaterialSubset> MaterialSubsets;

    FVector BoundingBoxMin;
    FVector BoundingBoxMax;
};
