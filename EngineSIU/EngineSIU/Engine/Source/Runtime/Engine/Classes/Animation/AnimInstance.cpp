#include "AnimInstance.h"
#include "Engine/Asset/AnimationAsset.h"
#include "Animation/AnimSequenceBase.h"
#include "UObject/Casts.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Math/Transform.h"

UAnimInstance::UAnimInstance()
{
}

UAnimInstance::~UAnimInstance()
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaTime)
{
    for (const auto& PlaybackContext : AnimationPlaybackContexts)
    {
        UAnimSequence* AnimSequenceBase = Cast<UAnimSequence>(PlaybackContext->AnimationAsset);
        
        if (!AnimSequenceBase)
            continue;

        for (const auto& Notify : AnimSequenceBase->GetAnimNotifies())
        {
            if (PlaybackContext->PlayRate > 0)
            {
                if (PlaybackContext->PreviousTime <= Notify.TriggerTime && PlaybackContext->PlaybackTime >= Notify.TriggerTime)
                {
                    SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Start);
                }
                else if (PlaybackContext->PreviousTime > Notify.TriggerTime && Notify.Duration > 0)
                {
                    float EndTime = Notify.TriggerTime + Notify.Duration;
                    if (PlaybackContext->PlaybackTime - Notify.TriggerTime <= Notify.Duration)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Tick, DeltaTime);
                    }
                    else if(PlaybackContext->PreviousTime <= EndTime && PlaybackContext->PlaybackTime >= EndTime)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::End);
                    }
                }
            }
            //역방향 재생시 반대로 계산
            else if (PlaybackContext->PlayRate < 0)
            {
                if (PlaybackContext->PreviousTime >= Notify.TriggerTime && PlaybackContext->PlaybackTime <= Notify.TriggerTime)
                {
                    SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Start);
                }
                else if (PlaybackContext->PreviousTime < Notify.TriggerTime && Notify.Duration > 0)
                {
                    float EndTime = Notify.TriggerTime + Notify.Duration;
                    if (Notify.TriggerTime - PlaybackContext->PlaybackTime <= Notify.Duration)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Tick, DeltaTime);
                    }
                    else if (PlaybackContext->PreviousTime >= EndTime && PlaybackContext->PlaybackTime <= EndTime)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::End);
                    }
                }
            }
        }
    }
}

void UAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    for (auto& Context : AnimationPlaybackContexts)
    {
        if (!Context)
            continue;

        if (Context->bIsPlaying)
        {
            // 시간 로직
            Context->PreviousTime = Context->PlaybackTime;
            Context->PlaybackTime += Context->PlayRate * DeltaTime;

        }
        if (Context->bIsLooping)
        {
            Context->PlaybackTime = fmodf(Context->PlaybackTime, Context->AnimationLength);
        }
        else
        {
            Context->PlaybackTime = FMath::Clamp(Context->PlaybackTime, 0.f, Context->AnimationLength);
        }
    }
    TriggerAnimNotifies(DeltaTime);
}

void UAnimInstance::AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
{
    if (InAnimAsset)
    {
        bool bIsExist = false;
        for (const auto& PlaybackContext : AnimationPlaybackContexts)
        {
            if (PlaybackContext->AnimationAsset == InAnimAsset)
            {
                bIsExist = true;
                break;
            }
        }
        if (bIsExist)
        {
            return;
        }
        auto PlaybackContext = std::make_shared<FAnimationPlaybackContext>(InAnimAsset, IsLoop, InPlayRate, InStartPosition);
        AnimationPlaybackContexts.Add(PlaybackContext);
    }
}

FAnimationPlaybackContext* UAnimInstance::GetAnimationPlaybackContext(UAnimationAsset* InAnimAsset)
{
    for (auto& PlaybackContext : AnimationPlaybackContexts)
    {
        if (PlaybackContext->AnimationAsset == InAnimAsset)
            return PlaybackContext.get();
    }
    return nullptr;
}

void UAnimInstance::Initialize(USkeletalMeshComponent* MeshComponent)
{
    SkeletalMeshComponent = MeshComponent;
    AnimationPlaybackContexts.Empty();
}

void UAnimInstance::ClearAnimationPlaybackContexts()
{
    AnimationPlaybackContexts.Empty();
}

FTransform UAnimInstance::GetCurrentAnimatedTransform(UAnimationAsset* AnimInstance, FName BoneName)
{
    if (!AnimInstance)
        return FTransform();

    FAnimationPlaybackContext* Context = GetAnimationPlaybackContext(AnimInstance);
    if (Context)
    {
        float CurrentAnimTime = Context->PlaybackTime;

        const FBoneAnimationTrack* BoneTrack = nullptr;

        UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimInstance);
        if (!AnimSequence)
        {
            UE_LOG(ELogLevel::Warning, "GetCurrentTransform : Only Support AnimSequence Currently. Identity Transform Returned");
            return FTransform::Identity;
        }
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

        return FTransform::Identity;
    }
}

FAnimationPlaybackContext::FAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
    : AnimationAsset(InAnimAsset), bIsLooping(IsLoop), PlayRate(InPlayRate), StartPosition(InStartPosition)
{
    PreviousTime = 0.f;
    PlaybackTime = 0.f;
    UAnimSequence* AnimSequence = Cast<UAnimSequence>(InAnimAsset);

    AnimationLength = AnimSequence ? AnimSequence->GetPlayLength() : 0.f;
}
