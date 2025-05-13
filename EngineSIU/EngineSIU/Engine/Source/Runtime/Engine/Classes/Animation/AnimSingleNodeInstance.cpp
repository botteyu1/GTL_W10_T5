#include "AnimSingleNodeInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "UObject/Casts.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/ReferenceSkeleton.h"
#include "Animation/Skeleton.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{
}

UAnimSingleNodeInstance::~UAnimSingleNodeInstance()
{
}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);
    if (CurrentAsset && bIsPlaying)
    {
        FAnimationPlaybackContext* PlaybackContext = GetAnimationPlaybackContext(CurrentAsset);
        if (!PlaybackContext)
        {
            return;
        }

        float ElapsedTime = PlaybackContext->PlaybackTime;
        UpdateBone(ElapsedTime);
    }
}

void UAnimSingleNodeInstance::UpdateBone(float ElapsedTime)
{
    USkeletalMeshComponent* SkeletalMeshComponent = GetSkeletalMeshComponent();
    if (!SkeletalMeshComponent)
    {
        return;
    }

    USkeletalMesh* SkeletalMeshAsset = SkeletalMeshComponent->GetSkeletalMeshAsset();
    UAnimSequence* AnimSequence = Cast<UAnimSequence>(CurrentAsset);

    if (!AnimSequence || !SkeletalMeshAsset || !SkeletalMeshAsset->GetSkeleton() || !AnimSequence->GetAnimDataModel())
    {
        return;
    }

    const USkeleton* Skeleton = SkeletalMeshAsset->GetSkeleton();
    const TArray<FMeshBoneInfo>& SkeletonBones = Skeleton->GetReferenceSkeleton().RawRefBoneInfo;

    // !TODO : 여기를 FPoseContext등으로 수정해야 함
    TArray<FTransform>& BoneTransforms = SkeletalMeshComponent->BoneTransforms;
    TArray<FTransform>& BoneBindPoseTransforms = SkeletalMeshComponent->BoneBindPoseTransforms;

    if (BoneTransforms.Num() != SkeletonBones.Num())
    {
        BoneTransforms.SetNum(SkeletonBones.Num());
        for (int32 i = 0; i < SkeletonBones.Num(); ++i)
        {
            BoneTransforms[i] = FTransform::Identity;
        }
    }

    for (uint32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
    {
        const FName CurrentBoneName = SkeletonBones[BoneIdx].Name;
        FTransform AnimatedTransform = GetCurrentAnimatedTransform(CurrentAsset, CurrentBoneName);

        BoneTransforms[BoneIdx] = AnimatedTransform;
    }
}

void UAnimSingleNodeInstance::SetAnimationAsset(UAnimationAsset* InAnimationAsset, bool IsLoop, float InPlayRate, float InStartPosition)
{
    CurrentAsset = InAnimationAsset;
    AddAnimationPlaybackContext(CurrentAsset, IsLoop, InPlayRate, InStartPosition);
}

void UAnimSingleNodeInstance::PlayAnim(bool bIsLooping, float InPlayRate, float InStartPosition)
{
    if (CurrentAsset)
    {
        // 애니메이션 자산이 설정되어 있으면 재생
        SetPlaying(true);
        SetLooping(bIsLooping);
        //AnimSequencePlaybackContexts.Add(new FAnimSequencePlaybackContext());
    }
    else
    {
        // 애니메이션 자산이 없으면 정지
        StopAnim();
    }

}

void UAnimSingleNodeInstance::StopAnim()
{
    bIsPlaying = false;
    SetAnimationTime(0); //정지하고 시간 0으로 리셋
}

void UAnimSingleNodeInstance::SetPlaying(bool bInPlaying)
{
    bIsPlaying = bInPlaying;
    if (!bInPlaying)
    {
        StopAnim();
    }
    FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(CurrentAsset);
    if (Context)
    {
        Context->bIsPlaying = bInPlaying;
    }
}

bool UAnimSingleNodeInstance::IsPlaying() const
{
    return bIsPlaying;
}

void UAnimSingleNodeInstance::PauseAnim()
{
    bIsPlaying = false;
    FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(CurrentAsset);
    if (Context)
    {
        Context->bIsPlaying = false;
    }
}

void UAnimSingleNodeInstance::SetLooping(bool bInLooping)
{
    GetAnimationPlaybackContext(CurrentAsset)->bIsLooping = bInLooping;
}

bool UAnimSingleNodeInstance::IsLooping()
{
    if (CurrentAsset && GetAnimationPlaybackContext(CurrentAsset)->bIsLooping)
    {
        return true;
    }
    return false;
}

void UAnimSingleNodeInstance::SetAnimationTime(float InTime)
{
    if (CurrentAsset)
    {
        FAnimationPlaybackContext* PlaybackContext = GetAnimationPlaybackContext(CurrentAsset);
        if (!PlaybackContext)
        {
            return;
        }
        PlaybackContext->PreviousTime = PlaybackContext->PlaybackTime;
        PlaybackContext->PlaybackTime = InTime;
        float Diff = PlaybackContext->PlaybackTime - PlaybackContext->PreviousTime;
        
        if (FMath::Abs(Diff) >= PlaybackContext->AnimationLength - 0.01f)
        {
            if (Diff < 0)
            {
                PlaybackContext->PlayRate = 1;
            }
            else if (Diff > 0)
            {
                PlaybackContext->PlayRate = -1;
            }
        }
        else
        {
            if (Diff >= 0)
                PlaybackContext->PlayRate = 1;
            else
                PlaybackContext->PlayRate = -1;
        }
        TriggerAnimNotifies(0);
        UpdateBone(InTime);
    }
}

void UAnimSingleNodeInstance::AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
{
    ClearAnimationPlaybackContexts(); // Single이니까 Clear 시킨 후 추가
    Super::AddAnimationPlaybackContext(InAnimAsset, IsLoop, InPlayRate, InStartPosition);
}
