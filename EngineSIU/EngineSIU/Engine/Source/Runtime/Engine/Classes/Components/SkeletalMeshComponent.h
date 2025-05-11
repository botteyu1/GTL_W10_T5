#pragma once
#include "SkinnedMeshComponent.h"
#include "Engine/AssetManager.h"

class UAnimSequence;
class USkeletalMesh;
struct FAnimNotifyEvent;

class USkeletalMeshComponent : public USkinnedMeshComponent
{
    DECLARE_CLASS(USkeletalMeshComponent, USkinnedMeshComponent)

public:
    USkeletalMeshComponent();
    virtual ~USkeletalMeshComponent() override;

    void TickComponent(float DeltaTime) override;

    USkeletalMesh* GetSkeletalMeshAsset() const { return SkeletalMeshAsset; }

    void SetSkeletalMeshAsset(USkeletalMesh* InSkeletalMeshAsset);


    TArray<FTransform> BoneTransforms;

    TArray<FTransform> BoneBindPoseTransforms; // 원본 BindPose에서 복사해온 에디팅을 위한 Transform

    void GetCurrentGlobalBoneMatrices(TArray<FMatrix>& OutBoneMatrices) const;

    void SetAnimationEnabled(bool bEnable);

    void ProcessAnimation(float DeltaTime);

    void ProcessAnimation2(float DeltaTime);

    UAnimSequence* GetAnimSequence() const { return AnimSequence; }
    void SetAnimSequence(UAnimSequence* InAnimSequence) { AnimSequence = InAnimSequence; };
       
    void HandleAnimNotify(const FAnimNotifyEvent& Notify);

private:
    // !TODO : 애니메이션 인스턴스 로직으로 변경
    UAnimSequence* AnimSequence = nullptr;
    USkeletalMesh* SkeletalMeshAsset = nullptr;

    float ElapsedTime = 0.f;

    bool bPlayAnimation = false;
};
