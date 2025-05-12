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
    if (CurrentAsset && bIsPlaying)
    {
        std::shared_ptr<FAnimationPlaybackContext>& PlaybackContext = GetAnimationPlaybackContext(CurrentAsset);
        if (PlaybackContext->PlayRate > 0)
        {
            PlaybackContext->PreviousTime = PlaybackContext->PlaybackTime;
            PlaybackContext->PlaybackTime += DeltaTime;
        }
        else if (PlaybackContext->PlayRate < 0)
        {
            PlaybackContext->PreviousTime = PlaybackContext->PlaybackTime;
            PlaybackContext->PlaybackTime -= DeltaTime;
        }
        PlaybackContext->PlaybackTime = fmodf(PlaybackContext->PlaybackTime, PlaybackContext->AnimationLength);

        float ElapsedTime = PlaybackContext->PlaybackTime;
        UpdateBone(ElapsedTime);
        TriggerAnimNotifies(DeltaTime);
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

    UAnimDataModel* AnimDataModel = AnimSequence->GetAnimDataModel();

    float CurrentAnimTime = ElapsedTime;
    if (AnimSequence->GetPlayLength() > 0.f)
    {
        CurrentAnimTime = FMath::Fmod(ElapsedTime, AnimSequence->GetPlayLength());
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

    TArray<FTransform> LocalAnimatedTransforms;
    LocalAnimatedTransforms.SetNum(SkeletonBones.Num());

    // 본 별 로컬 변환 계산
    for (int32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
    {
        const FName CurrentBoneName = SkeletonBones[BoneIdx].Name;
        const FBoneAnimationTrack* BoneTrack = nullptr;

        // 트랙 찾기
        for (const FBoneAnimationTrack& Track : AnimDataModel->GetBoneAnimationTracks())
        {
            if (Track.Name == CurrentBoneName)
            {
                BoneTrack = &Track;
                break;
            }
        }

        if (BoneTrack && !BoneTrack->InternalTrack.IsEmpty())
        {
            // 애니메이션의 Transform을 그대로 사용하는 것인지, 곱해주는 것인지 판단해야 함
            FVector FinalPos = FVectorKey::Interpolate(BoneTrack->InternalTrack.PosKeys, CurrentAnimTime);
            FQuat FinalRot = FQuatKey::Interpolate(BoneTrack->InternalTrack.RotKeys, CurrentAnimTime);
            FVector FinalScale = FVectorKey::Interpolate(BoneTrack->InternalTrack.ScaleKeys, CurrentAnimTime);

            // scale은 0이 되지 못하도록 예외
            if (FinalScale.IsZero())
            {
                FinalScale = FVector(1.f, 1.f, 1.f);
            }

            LocalAnimatedTransforms[BoneIdx] = FTransform(FinalRot, FinalPos, FinalScale);
        }
        else
        {
            // 애니메이션 트랙이 없거나 비어있으면, 해당 뼈의 로컬 바인드 포즈 사용
            // USkeleton 또는 FBoneInfo에 로컬 바인드 포즈 정보가 있어야 함
            // 예: LocalAnimatedTransforms[BoneIdx] = SkeletonBones[BoneIdx].LocalBindTransform;
            // BoneBindPoseTransforms가 로컬 공간 기준이고, 인덱스가 일치한다면 사용 가능

            LocalAnimatedTransforms[BoneIdx] = BoneBindPoseTransforms[BoneIdx]; // 안전장치
        }
    }

    for (uint32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
    {
        uint32 ParentIndex = SkeletonBones[BoneIdx].ParentIndex;

        BoneTransforms[BoneIdx] = LocalAnimatedTransforms[BoneIdx];
    }

}

void UAnimSingleNodeInstance::SetAnimationAsset(UAnimationAsset* InAnimationAsset, bool IsLoop, float InPlayRate, float InStartPosition)
{
    CurrentAsset = InAnimationAsset;
    AddAnimationPlaybackContext(CurrentAsset);
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
}

bool UAnimSingleNodeInstance::IsPlaying() const
{
    return bIsPlaying;
}

void UAnimSingleNodeInstance::PauseAnim()
{
    bIsPlaying = false;
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
        std::shared_ptr<FAnimationPlaybackContext>& PlaybackContext = GetAnimationPlaybackContext(CurrentAsset);
        PlaybackContext->PreviousTime = PlaybackContext->PlaybackTime;
        if (PlaybackContext->PreviousTime < InTime)
            PlaybackContext->PlayRate = 1;
        else
            PlaybackContext->PlayRate = -1;
        PlaybackContext->PlaybackTime = InTime;
        TriggerAnimNotifies(0);
        UpdateBone(InTime);
    }
}

void UAnimSingleNodeInstance::AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
{
    ClearAnimationPlaybackContexts(); // Single이니까 Clear 시킨 후 추가
    Super::AddAnimationPlaybackContext(InAnimAsset, IsLoop, InPlayRate, InStartPosition);
}
