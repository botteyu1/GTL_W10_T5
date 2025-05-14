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

UObject* UAnimInstance::Duplicate(UObject* InOuter)
{
    ThisClass* NewObject = Cast<ThisClass>(Super::Duplicate(InOuter));
    return NewObject;
}

void UAnimInstance::TriggerAnimNotifies(float DeltaTime)
{
    for (const auto& PlaybackContext : AnimationPlaybackContexts)
    {
        UAnimSequence* AnimSequenceBase = Cast<UAnimSequence>(PlaybackContext->AnimationAsset);
        
        if (!AnimSequenceBase)
            continue;

        for (auto& Notify : AnimSequenceBase->GetAnimNotifies())
        {
            if (Notify.TriggerTime < 0.005f)
                Notify.TriggerTime = 0.005f;
            if (PlaybackContext->PlayRate > 0)
            {
                if (PlaybackContext->PreviousTime <= Notify.TriggerTime && PlaybackContext->PlaybackTime >= Notify.TriggerTime)
                {
                    SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Start);
                }
                else if (PlaybackContext->PlaybackTime >= Notify.TriggerTime && Notify.Duration > 0)
                {
                    float EndTime = Notify.TriggerTime + Notify.Duration;
                    float AnimLength = AnimSequenceBase->GetPlayLength() - 0.005f;
                    EndTime = FMath::Min(EndTime, AnimLength);
                    UE_LOG(ELogLevel::Display, "\nPlaybackTime: %f\nPrevious Time: %f\nEndTime: %f", PlaybackContext->PlaybackTime, PlaybackContext->PreviousTime, EndTime);
                    if (PlaybackContext->PreviousTime <= EndTime && PlaybackContext->PlaybackTime >= EndTime)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::End);
                    }
                    else if (PlaybackContext->PlaybackTime - Notify.TriggerTime <= Notify.Duration)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Tick, DeltaTime);
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
                else if (PlaybackContext->PreviousTime >= Notify.TriggerTime && Notify.Duration > 0)
                {
                    float EndTime = Notify.TriggerTime + Notify.Duration;
                    EndTime = FMath::Min(EndTime, AnimSequenceBase->GetPlayLength() - 0.005f);
                    if (PlaybackContext->PreviousTime >= EndTime && PlaybackContext->PlaybackTime <= EndTime)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::End);
                    }
                    else if (Notify.TriggerTime - PlaybackContext->PlaybackTime <= Notify.Duration)
                    {
                        SkeletalMeshComponent->HandleAnimNotify(Notify, ENotifyState::Tick, DeltaTime);
                    }
                }
            }
        }
    }
}

void UAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    // Remove contexts that are marked for removal
    AnimationPlaybackContexts.RemoveAll([](const auto& ContextToRemove)
        {
            if (!ContextToRemove)
            {
                return false;
            }
            return ContextToRemove->bIsRemove;
        }
    );

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
            // 애니메이션이 끝나면 Contexts 배열에서 제거
            if (Context->PlaybackTime >= Context->AnimationLength)
            {
                Context->bIsRemove = true;
            }
            else
            {
                Context->PlaybackTime = FMath::Clamp(Context->PlaybackTime, 0.f, Context->AnimationLength);
            }
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

void UAnimInstance::RemoveAnimationPlaybackContext(UAnimationAsset* InAnimAsset)
{
    AnimationPlaybackContexts.RemoveAll([&](const auto& ContextToRemove)
        {
            return ContextToRemove->AnimationAsset == InAnimAsset;
        }
    );
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
}

void UAnimInstance::ClearAnimationPlaybackContexts()
{
    AnimationPlaybackContexts.Empty();
}

void UAnimInstance::PauseAnimations()
{
    for (auto& PlaybackContext : AnimationPlaybackContexts)
    {
        PlaybackContext->bIsPlaying = false;
    }
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
