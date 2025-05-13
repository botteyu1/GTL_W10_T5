#include "BlendAnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Math/Transform.h"
#include "UObject/Casts.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include <ReferenceSkeleton.h>
#include "Animation/Skeleton.h"

UBlendAnimInstance::UBlendAnimInstance()
{
}

UBlendAnimInstance::~UBlendAnimInstance()
{
}

void UBlendAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    if (!bIsPlaying)
        return;
    if (AnimA && AnimB)
    {
        FAnimationPlaybackContext* PlaybackContextA = GetAnimationPlaybackContext(AnimA);
        FAnimationPlaybackContext* PlaybackContextB = GetAnimationPlaybackContext(AnimB);

        USkeletalMeshComponent* SkeletalMeshComponent = GetSkeletalMeshComponent();

        if (!SkeletalMeshComponent || !PlaybackContextA || !PlaybackContextB)
            return;

        const USkeletalMesh* SkeletalMeshAsset = SkeletalMeshComponent->GetSkeletalMeshAsset();
        if (!SkeletalMeshAsset)
            return;
        const USkeleton* Skeleton = SkeletalMeshAsset->GetSkeleton();
        const TArray<FMeshBoneInfo>& SkeletonBones = Skeleton->GetReferenceSkeleton().RawRefBoneInfo;

        TArray<FTransform>& BoneTransforms = SkeletalMeshComponent->BoneTransforms;

        TArray<FTransform> LocalAnimatedTransforms;
        LocalAnimatedTransforms.SetNum(SkeletonBones.Num());

        for (int32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
        {
            const FName CurrentBoneName = SkeletonBones[BoneIdx].Name;
            FTransform TransformA = GetCurrentAnimatedTransform(AnimA, CurrentBoneName);
            FTransform TransformB = GetCurrentAnimatedTransform(AnimB, CurrentBoneName);

            FTransform BlendedTransform;
            BlendedTransform.Blend(TransformA, TransformB, BlendAlpha);

            LocalAnimatedTransforms[BoneIdx] = BlendedTransform;
        }

        for (uint32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
        {
            uint32 ParentIndex = SkeletonBones[BoneIdx].ParentIndex;

            BoneTransforms[BoneIdx] = LocalAnimatedTransforms[BoneIdx];
        }
    }
}

void UBlendAnimInstance::SetAnimSequences(UAnimSequence* InAnimA, UAnimSequence* InAnimB)
{
    if (InAnimA)
    {
        AnimA = InAnimA;
        AddAnimationPlaybackContext(InAnimA, true);
    }
    if (InAnimB)
    {
        AnimB = InAnimB;
        AddAnimationPlaybackContext(InAnimB, true);
    }
}


void UBlendAnimInstance::SetAnimationEnabled(bool bEnable)
{
    bIsPlaying = bEnable;
    if (AnimA)
    {
        FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(AnimA);
        if (Context)
        {
            Context->bIsPlaying = bEnable;
        }
    }
    if (AnimB)
    {
        FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(AnimB);
        if (Context)
        {
            Context->bIsPlaying = bEnable;
        }
    }
}

