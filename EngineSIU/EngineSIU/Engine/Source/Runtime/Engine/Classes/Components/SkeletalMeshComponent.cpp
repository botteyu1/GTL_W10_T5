
#include "SkeletalMeshComponent.h"

#include "ReferenceSkeleton.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimDataModel.h"
#include "UObject/ObjectFactory.h"
USkeletalMeshComponent::USkeletalMeshComponent()
{
    AnimSequence = FObjectFactory::ConstructObject<UAnimSequence>(this);
}

USkeletalMeshComponent::~USkeletalMeshComponent()
{
    if (AnimSequence)
    {
        delete AnimSequence;
        AnimSequence = nullptr;
    }
}



void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    USkinnedMeshComponent::TickComponent(DeltaTime);

    //ProcessAnimation(DeltaTime);
    ProcessAnimation2(DeltaTime);
}

void USkeletalMeshComponent::SetSkeletalMeshAsset(USkeletalMesh* InSkeletalMeshAsset)
{
    SkeletalMeshAsset = InSkeletalMeshAsset;

    BoneTransforms.Empty();
    BoneBindPoseTransforms.Empty();
    
    const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();
    for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
    {
        BoneTransforms.Add(RefSkeleton.RawRefBonePose[i]);
        BoneBindPoseTransforms.Add(RefSkeleton.RawRefBonePose[i]);
    }
}

void USkeletalMeshComponent::GetCurrentGlobalBoneMatrices(TArray<FMatrix>& OutBoneMatrices) const
{
    const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();
    const TArray<FTransform>& BindPose = RefSkeleton.RawRefBonePose; // 로컬
    const int32 BoneNum = RefSkeleton.RawRefBoneInfo.Num();

    // 1. 현재 애니메이션 본 행렬 계산 (계층 구조 적용)
    OutBoneMatrices.Empty();
    OutBoneMatrices.SetNum(BoneNum);

    for (int32 BoneIndex = 0; BoneIndex < BoneNum; ++BoneIndex)
    {
        // 현재 본의 로컬 변환
        FTransform CurrentLocalTransform = BoneTransforms[BoneIndex];
        FMatrix LocalMatrix = CurrentLocalTransform.ToMatrixWithScale(); // FTransform -> FMatrix
        
        // 부모 본의 영향을 적용하여 월드 변환 구성
        int32 ParentIndex = RefSkeleton.RawRefBoneInfo[BoneIndex].ParentIndex;
        if (ParentIndex != INDEX_NONE)
        {
            // 로컬 변환에 부모 월드 변환 적용
            LocalMatrix = LocalMatrix * OutBoneMatrices[ParentIndex];
        }
        
        // 결과 행렬 저장
        OutBoneMatrices[BoneIndex] = LocalMatrix;
    }
}

void USkeletalMeshComponent::SetAnimationEnabled(bool bEnable)
{
    bPlayAnimation = bEnable;

    if (!bPlayAnimation)
    {
        if (SkeletalMeshAsset && SkeletalMeshAsset->GetSkeleton())
        {
            const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();
            BoneTransforms = RefSkeleton.RawRefBonePose;
        }
    }
}

void USkeletalMeshComponent::ProcessAnimation(float DeltaTime)
{
    if (bPlayAnimation)
    {
        ElapsedTime += DeltaTime;
    }


    BoneTransforms = BoneBindPoseTransforms;

    if (bPlayAnimation && AnimSequence && SkeletalMeshAsset && SkeletalMeshAsset->GetSkeleton())
    {
        const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();

        const int32 AnimationFrameRate = AnimSequence->FrameRate;
        const int32 AnimationLength = AnimSequence->NumFrames;

        const float TargetKeyFrame = ElapsedTime * static_cast<float>(AnimationFrameRate);
        const int32 CurrentKey = static_cast<int32>(TargetKeyFrame) % AnimationLength;
        const int32 NextKey = (CurrentKey + 1) % AnimationLength;
        const float Alpha = TargetKeyFrame - static_cast<float>(static_cast<int32>(TargetKeyFrame)); // [0 ~ 1]

        TMap<int32, FTransform> CurrentFrameTransforms = AnimSequence->Anim[CurrentKey];
        TMap<int32, FTransform> NextFrameTransforms = AnimSequence->Anim[NextKey];

        for (auto& [BoneIdx, CurrentTransform] : CurrentFrameTransforms)
        {
            // 다음 키프레임에 해당 본 데이터가 있는지 확인
            if (NextFrameTransforms.Contains(BoneIdx))
            {
                FTransform NextTransform = NextFrameTransforms[BoneIdx];
                // 두 트랜스폼 사이를 Alpha 비율로 선형 보간
                FTransform InterpolatedTransform = FTransform::Identity;
                InterpolatedTransform.Blend(CurrentTransform, NextTransform, Alpha);

                // 보간된 트랜스폼 적용 (로컬 포즈 * 애니메이션 트랜스폼)
                BoneTransforms[BoneIdx] = BoneBindPoseTransforms[BoneIdx] * InterpolatedTransform;
            }
            else
            {
                // 다음 키프레임에 본 데이터가 없으면 현재 트랜스폼만 사용
                BoneTransforms[BoneIdx] = BoneBindPoseTransforms[BoneIdx] * CurrentTransform;
            }
        }
    }
}

void USkeletalMeshComponent::ProcessAnimation2(float DeltaTime)
{
    if (!bPlayAnimation || !AnimSequence || !SkeletalMeshAsset || !SkeletalMeshAsset->GetSkeleton() || !AnimSequence->GetAnimDataModel()) 
    {
        return;
    }

    UAnimDataModel* AnimDataModel = AnimSequence->GetAnimDataModel();
    ElapsedTime += DeltaTime;

    float CurrentAnimTime = ElapsedTime;
    if (AnimSequence->GetPlayLength() > 0.f)
    {
        CurrentAnimTime = FMath::Fmod(ElapsedTime, AnimSequence->GetPlayLength());
    }

    const USkeleton* Skeleton = SkeletalMeshAsset->GetSkeleton();
    const TArray<FMeshBoneInfo>& SkeletonBones = Skeleton->GetReferenceSkeleton().RawRefBoneInfo;

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

            LocalAnimatedTransforms[BoneIdx] = FTransform(FinalRot, FinalPos, FinalScale);
        }
        else
        {
            // 애니메이션 트랙이 없거나 비어있으면, 해당 뼈의 로컬 바인드 포즈 사용
            // USkeleton 또는 FBoneInfo에 로컬 바인드 포즈 정보가 있어야 함
            // 예: LocalAnimatedTransforms[BoneIdx] = SkeletonBones[BoneIdx].LocalBindTransform;
            // BoneBindPoseTransforms가 로컬 공간 기준이고, 인덱스가 일치한다면 사용 가능

            LocalAnimatedTransforms[BoneIdx] = FTransform::Identity; // 안전장치
        }
    }

    for (uint32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
    {
        uint32 ParentIndex = SkeletonBones[BoneIdx].ParentIndex;

        if (ParentIndex != INDEX_NONE && ParentIndex >= 0 && ParentIndex < BoneIdx)
        {
            // !NOTE : 곱셈 순서 확인!!!
            BoneTransforms[BoneIdx] = LocalAnimatedTransforms[BoneIdx];
        }
        else
        {
            // 부모가 없거나 루트 본인 경우
            //BoneTransforms[BoneIdx] = BoneBindPoseTransforms[BoneIdx];
            BoneTransforms[BoneIdx] = LocalAnimatedTransforms[BoneIdx];
        }
    }
}
