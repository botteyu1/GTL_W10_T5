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
    if (!bIsPlaying)
        return;
    if (AnimA && AnimB)
    {
        // !TODO : 구데기 코드 고치기
        FAnimationPlaybackContext* PlaybackContextA = GetAnimationPlaybackContext(AnimA);
        FAnimationPlaybackContext* PlaybackContextB = GetAnimationPlaybackContext(AnimB);

        USkeletalMeshComponent* SkeletalMeshComponent = GetSkeletalMeshComponent();

        PlaybackContextA->PreviousTime = PlaybackContextA->PlaybackTime;
        PlaybackContextA->PlaybackTime += DeltaTime;

        PlaybackContextB->PreviousTime = PlaybackContextB->PlaybackTime;
        PlaybackContextB->PlaybackTime += DeltaTime;

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
            FTransform TransformA = GetCurrentTransform(AnimA, CurrentBoneName);
            FTransform TransformB = GetCurrentTransform(AnimB, CurrentBoneName);

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

FTransform UBlendAnimInstance::GetCurrentTransform(UAnimSequence* AnimSequence, FName BoneName)
{
    if(!AnimSequence)
        return FTransform();

    FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(Cast<UAnimationAsset>(AnimSequence));
    if(Context)
    {
        float CurrentAnimTime = Context->PlaybackTime;
        // !TODO : Looping관련 로직
        if (AnimSequence->GetPlayLength() > 0.f)
        {
            CurrentAnimTime = FMath::Fmod(CurrentAnimTime, AnimSequence->GetPlayLength());
        }

        const FBoneAnimationTrack* BoneTrack = nullptr;

        // 트랙 찾기
        for (const FBoneAnimationTrack& Track : AnimSequence->GetAnimDataModel()->GetBoneAnimationTracks())
        {
            if (Track.Name == BoneName)
            {
                BoneTrack = &Track;
                break;
            }
        }

        if (BoneTrack && !BoneTrack->InternalTrack.IsEmpty())
        {
            FVector FinalPos = FVectorKey::Interpolate(BoneTrack->InternalTrack.PosKeys, CurrentAnimTime);
            FQuat FinalRot = FQuatKey::Interpolate(BoneTrack->InternalTrack.RotKeys, CurrentAnimTime);
            FVector FinalScale = FVectorKey::Interpolate(BoneTrack->InternalTrack.ScaleKeys, CurrentAnimTime);

            // scale은 0이 되지 못하도록 예외
            if (FinalScale.IsZero())
            {
                FinalScale = FVector(1.f, 1.f, 1.f);
            }

            return FTransform(FinalRot, FinalPos, FinalScale);
        }

        return FTransform();
    }
}
