#pragma once
#include "SkinnedMeshComponent.h"
#include "Engine/AssetManager.h"

class UAnimSequence;
class USkeletalMesh;

class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

public:
    USkeletalMeshComponent();
    virtual ~USkeletalMeshComponent() override;

    void TickComponent(float DeltaTime) override;

    USkeletalMesh* GetSkeletalMeshAsset() const { return SkeletalMeshAsset; }

    void SetSkeletalMeshAsset(USkeletalMesh* InSkeletalMeshAsset);

    UAnimSequence* AnimSequence = nullptr;

    TArray<FTransform> BoneTransforms;

    TArray<FTransform> BoneBindPoseTransforms; // 원본 BindPose에서 복사해온 에디팅을 위한 Transform

    void GetCurrentGlobalBoneMatrices(TArray<FMatrix>& OutBoneMatrices) const;

    void SetAnimationEnabled(bool bEnable);

    void ProcessAnimation(float DeltaTime);

    void ProcessAnimation2(float DeltaTime);

    void SetAnimationTime(float InTime) { ElapsedTime = InTime; }

private:
    
    USkeletalMesh* SkeletalMeshAsset = nullptr;

    float ElapsedTime = 0.f; // TODO anim인스턴스로 이동 해야됨

    bool bPlayAnimation = false;
};
