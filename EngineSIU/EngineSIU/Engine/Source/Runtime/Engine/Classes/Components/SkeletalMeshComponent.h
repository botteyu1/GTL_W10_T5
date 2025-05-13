#pragma once
#include "SkinnedMeshComponent.h"
#include "Engine/AssetManager.h"

class UAnimInstance;
class UAnimSequence;
class USkeletalMesh;
struct FAnimNotifyEvent;
class UAnimSingleNodeInstance;
class UBlendAnimInstance;

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
       
    void HandleAnimNotify(const FAnimNotifyEvent& Notify);

    void SetAnimationTime(float InTime); 

    void SetAnimInstanceClass(UClass* InstanceClass);

    UAnimInstance* GetAnimInstance() const { return AnimInstance; }

private:
    USkeletalMesh* SkeletalMeshAsset = nullptr;
    UAnimInstance* AnimInstance = nullptr;

    UAnimInstance* CachedAnimSingleNodeInstance = nullptr;
    UAnimInstance* CachedBlendAnimInstance = nullptr;

    float ElapsedTime = 0.f; // TODO anim인스턴스로 이동 해야됨

    bool bPlayAnimation = false;
};
