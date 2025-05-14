#include "FSMAnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/AssetManager.h"
#include "Math/Transform.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"

UFSMAnimInstance::UFSMAnimInstance()
{
}

UFSMAnimInstance::~UFSMAnimInstance()
{
}

void UFSMAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    if (!bIsPlaying)
        return;

    Super::NativeUpdateAnimation(DeltaTime);

    TArray<FTransform> FinalLocalSpacePose;
    USkeletalMeshComponent* SkelComp = GetSkeletalMeshComponent();

    if (!SkelComp || !SkelComp->GetSkeletalMeshAsset() || !SkelComp->GetSkeletalMeshAsset()->GetSkeleton())
        return;

    TArray<FTransform>& SourceBindPose = SkelComp->BoneTransforms; // 원본 포즈

    if (bIsBlending)
    {
        ElapsedBlendTime += DeltaTime;
        CurrentBlendAlpha = FMath::Clamp(ElapsedBlendTime / BlendDuration, 0.0f, 1.0f);

        const TArray<FMeshBoneInfo>& SkeletonBones = SkelComp->GetSkeletalMeshAsset()->GetSkeleton()->GetReferenceSkeleton().RawRefBoneInfo;
        FinalLocalSpacePose.SetNum(SkeletonBones.Num());

        for (int32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
        {
            const FName BoneName = SkeletonBones[BoneIdx].Name;
            FTransform TransformA = FTransform::Identity; // 소스 포즈
            FTransform TransformB = FTransform::Identity; // 타겟 포즈

            if (bIsBlendingFromSnapshot && SourceSnapshotPose.IsValidIndex(BoneIdx))
            {
                TransformA = SourceSnapshotPose[BoneIdx];
            }

            if (TargetAnimSequence)
            {
                FAnimationPlaybackContext* TargetContext = GetAnimationPlaybackContext(TargetAnimSequence);
                float TargetTime = TargetContext ? TargetContext->PlaybackTime : 0.f;
                TransformB = GetCurrentAnimatedTransform(TargetAnimSequence, BoneName);
            }

            FTransform BlendedBoneTransform;
            BlendedBoneTransform.Blend(TransformA, TransformB, CurrentBlendAlpha);
            FinalLocalSpacePose[BoneIdx] = BlendedBoneTransform;
        }

        if (CurrentBlendAlpha >= 1.0f) // 블렌드 완료
        {
            bIsBlending = false;
            bIsBlendingFromSnapshot = false;

            SourceSnapshotPose.Empty();

            // Source 애니메이션의 PlaybackContext를 제거
            if (BlendSourceAnimForSnapshot)
            {
                FAnimationPlaybackContext* SourceContext = GetAnimationPlaybackContext(BlendSourceAnimForSnapshot);
                if (SourceContext)
                {
                    SourceContext->bIsRemove = true;
                    //RemoveAnimationPlaybackContext(BlendSourceAnimForSnapshot);
                }
            }
        }
    }
    else if (CurrentAnimSequence) // 블렌딩 중이 아니고, 현재 상태 애니메이션이 있다면 단독 재생
    {
        USkeletalMeshComponent* SkelComp = GetSkeletalMeshComponent();
        if (!SkelComp || !SkelComp->GetSkeletalMeshAsset() || !SkelComp->GetSkeletalMeshAsset()->GetSkeleton())
            return;
        const TArray<FMeshBoneInfo>& SkeletonBones = SkelComp->GetSkeletalMeshAsset()->GetSkeleton()->GetReferenceSkeleton().RawRefBoneInfo;
        FinalLocalSpacePose.SetNum(SkeletonBones.Num());

        FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(CurrentAnimSequence);

        if (Context)
        {
            for (int32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
            {
                FinalLocalSpacePose[BoneIdx] = GetCurrentAnimatedTransform(CurrentAnimSequence, SkeletonBones[BoneIdx].Name);
            }
        }
        else
        {
            FinalLocalSpacePose = SourceBindPose;
        }
    }
    else // 재생할 애니메이션이 없음 (바인드 포즈)
    {
        USkeletalMeshComponent* SkelComp = GetSkeletalMeshComponent();
        if (!SkelComp || !SkelComp->GetSkeletalMeshAsset() || !SkelComp->GetSkeletalMeshAsset()->GetSkeleton())
            return;

        FinalLocalSpacePose = SourceBindPose;
    }

    ApplyPoseToComponent(FinalLocalSpacePose);
}

void UFSMAnimInstance::AddAnimState(const FString& StateName, const FString& AnimName)
{
    UAnimSequence* AnimSequence = UAssetManager::Get().GetAnimSequence(AnimName);

    if (AnimSequence)
    {
        AnimStateMap.Add(StateName, AnimSequence);

    }
    else
    {
        UE_LOG(ELogLevel::Warning, TEXT("Failed to find animation sequence: %s"), *AnimName);
    }
}

void UFSMAnimInstance::RemoveAnimState(const FString& StateName)
{
    if (AnimStateMap.Contains(StateName))
    {
        AnimStateMap.Remove(StateName);
    }
    else
    {
        UE_LOG(ELogLevel::Warning, TEXT("Failed to find animation state: %s"), *StateName);
    }
}

// !TODO : 추가 파라미터 적용
void UFSMAnimInstance::ChangeAnimState(const FString& TargetStateName, float BlendTime, bool bIsLooping)
{
    if (!AnimStateMap.Contains(TargetStateName))
    {
        UE_LOG(ELogLevel::Warning, TEXT("ChangeAnimState: Target state '%s' not found."), *TargetStateName);
        return;
    }
    UAnimSequence* NewTargetAnimSequence = AnimStateMap[TargetStateName];
    if (!NewTargetAnimSequence)
    {
        UE_LOG(ELogLevel::Warning, TEXT("ChangeAnimState: Animation for state '%s' is null."), *TargetStateName);
        return;
    }

    if (BlendTime <= 0.0f) // 블렌드 없이 즉시 변경
    {
        CurrentAnimSequence = NewTargetAnimSequence;
        TargetAnimSequence = nullptr; // 이전 블렌드 타겟 정리
        bIsBlending = false;
        bIsBlendingFromSnapshot = false;
        SourceSnapshotPose.Empty();
        ElapsedBlendTime = 0.0f;
        CurrentBlendAlpha = 0.0f;

        // 새 애니메이션의 PlaybackContext 설정 (루핑, 재생 속도 등은 기본값 사용 또는 파라미터화)
        // AddAnimationPlaybackContext는 기존 컨텍스트를 지우고 새로 추가하거나, 업데이트해야 함
        AddAnimationPlaybackContext(CurrentAnimSequence, bIsLooping, 1.0f, 0.0f);
    }
    else // 블렌딩 시작
    {
        if (bIsPlaying && (bIsBlending || CurrentAnimSequence != nullptr)) // 재생 중이고, 블렌딩 중이거나 현재 애니메이션이 있을 때만 스냅샷 의미 있음
        {
            CaptureCurrentPoseAsSnapshot(); // 현재 포즈를 다음 블렌드의 소스로 캡처
        }
        else // 현재 아무것도 재생 중이 아니거나, 첫 애니메이션 설정 시
        {
            bIsBlendingFromSnapshot = false; // 스냅샷 없음 (바인드 포즈에서 시작하는 것처럼)
            SourceSnapshotPose.Empty();
        }

        CurrentAnimSequence = NewTargetAnimSequence; // FSM의 논리적 현재 상태는 새 타겟
        TargetAnimSequence = NewTargetAnimSequence;  // 블렌드의 실제 타겟
        BlendDuration = BlendTime;
        ElapsedBlendTime = 0.0f;
        CurrentBlendAlpha = 0.0f;
        bIsBlending = true;

        // 새 타겟 애니메이션의 PlaybackContext 설정
        AddAnimationPlaybackContext(TargetAnimSequence, bIsLooping, 1.0f, 0.0f);
    }
    bIsPlaying = true; // 상태 변경 시 재생 시작 (필요에 따라 조절)
}

void UFSMAnimInstance::SetPlaying(bool bInPlaying)
{
    for (auto& PlaybackContext : AnimationPlaybackContexts)
    {
        PlaybackContext->bIsPlaying = bInPlaying;
    }
    bIsPlaying = bInPlaying;
}

void UFSMAnimInstance::CaptureCurrentPoseAsSnapshot()
{
    USkeletalMeshComponent* SkeletalMeshComponent = GetSkeletalMeshComponent();
    if (!SkeletalMeshComponent || !SkeletalMeshComponent->GetSkeletalMeshAsset() || !SkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton())
    {
        SourceSnapshotPose.Empty();
        bIsBlendingFromSnapshot = false;
        return;
    }

    const TArray<FMeshBoneInfo>& SkeletonBones = SkeletalMeshComponent->GetSkeletalMeshAsset()->GetSkeleton()->GetReferenceSkeleton().RawRefBoneInfo;
    SourceSnapshotPose.SetNum(SkeletonBones.Num());

    if (bIsBlending)// 블렌딩 중
    {
        BlendSourceAnimForSnapshot = bIsBlendingFromSnapshot ? nullptr : CurrentAnimSequence; // 이전 상태의 애니메이션이 소스였을 것
        BlendTargetAnimForSnapshot = TargetAnimSequence;
        AlphaForSnapshot = CurrentBlendAlpha;

        UAnimSequence* SourceForCurrentBlend = bIsBlendingFromSnapshot ? nullptr : CurrentAnimSequence;
        TArray<FTransform>* PreviousSnapshot = bIsBlendingFromSnapshot ? &SourceSnapshotPose : nullptr;

        for (int32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
        {
            const FName BoneName = SkeletonBones[BoneIdx].Name;
            FTransform TransformA = FTransform::Identity;
            if (PreviousSnapshot && PreviousSnapshot->IsValidIndex(BoneIdx))
            {
                TransformA = (*PreviousSnapshot)[BoneIdx];
            }
            else if (SourceForCurrentBlend)
            {
                FAnimationPlaybackContext* SrcContext = GetAnimationPlaybackContext(SourceForCurrentBlend);
                TransformA = GetCurrentAnimatedTransform(SourceForCurrentBlend, BoneName);
            }

            FTransform TransformB = FTransform::Identity;
            if (TargetAnimSequence)
            {
                FAnimationPlaybackContext* TargetContext = GetAnimationPlaybackContext(TargetAnimSequence);
                TransformB = GetCurrentAnimatedTransform(TargetAnimSequence, BoneName);
            }

            FTransform BlendedSnapshotPose;
            BlendedSnapshotPose.Blend(TransformA, TransformB, CurrentBlendAlpha);
            SourceSnapshotPose[BoneIdx] = BlendedSnapshotPose;
        }
    }
    else if (CurrentAnimSequence) // 단일 애니메이션 재생중
    {
        BlendSourceAnimForSnapshot = CurrentAnimSequence;
        BlendTargetAnimForSnapshot = nullptr;
        AlphaForSnapshot = 0.0f; // 또는 1.0f (의미에 따라)

        for (int32 BoneIdx = 0; BoneIdx < SkeletonBones.Num(); ++BoneIdx)
        {
            const FName BoneName = SkeletonBones[BoneIdx].Name;
            FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(CurrentAnimSequence);
            if (Context)
            {
                SourceSnapshotPose[BoneIdx] = GetCurrentAnimatedTransform(CurrentAnimSequence, BoneName);
            }
            else
            {
                SourceSnapshotPose = SkeletalMeshComponent->BoneTransforms; // 원본 바인드포즈
            }
        }
    }
    else
    {
        SourceSnapshotPose = SkeletalMeshComponent->BoneTransforms; // 원본 바인드포즈
    }

    bIsBlendingFromSnapshot = true; // 다음 블렌드는 이 스냅샷에서 시작
    
}

void UFSMAnimInstance::ApplyPoseToComponent(const TArray<FTransform>& AnimatedPose)
{
    USkeletalMeshComponent* SkelComp = GetSkeletalMeshComponent();

    if (!SkelComp || !SkelComp->GetSkeletalMeshAsset() || !SkelComp->GetSkeletalMeshAsset()->GetSkeleton()) return;

    const USkeleton* SkeletonAsset = SkelComp->GetSkeletalMeshAsset()->GetSkeleton();

    const TArray<FMeshBoneInfo>& RefSkeletonBones = SkeletonAsset->GetReferenceSkeleton().RawRefBoneInfo;

    if (AnimatedPose.Num() != RefSkeletonBones.Num()) return; // 포즈 데이터 크기 불일치

    TArray<FTransform>& ComponentSpaceBoneTransforms = SkelComp->BoneTransforms;

    if (ComponentSpaceBoneTransforms.Num() != RefSkeletonBones.Num())
    {
        ComponentSpaceBoneTransforms.SetNum(RefSkeletonBones.Num());
    }

    for (int32 BoneIdx = 0; BoneIdx < RefSkeletonBones.Num(); ++BoneIdx)
    {
        ComponentSpaceBoneTransforms[BoneIdx] = AnimatedPose[BoneIdx];
    }
}
