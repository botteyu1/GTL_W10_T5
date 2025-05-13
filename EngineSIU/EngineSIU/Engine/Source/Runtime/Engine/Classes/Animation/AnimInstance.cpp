#include "AnimInstance.h"
#include "Engine/Asset/AnimationAsset.h"
#include "Animation/AnimSequenceBase.h"
#include "UObject/Casts.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"

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
        
        for (const auto& Notify : AnimSequenceBase->GetAnimNotifies())
        {
            if (PlaybackContext->PlayRate > 0)
            {
                if (PlaybackContext->PreviousTime <= Notify.TriggerTime && PlaybackContext->PlaybackTime >= Notify.TriggerTime)
                {
                    SkeletalMeshComponent->StartAnimNotify(Notify);
                }
                else if (PlaybackContext->PreviousTime > Notify.TriggerTime && Notify.Duration > 0)
                {
                    float EndTime = Notify.TriggerTime + Notify.Duration;
                    if (PlaybackContext->PlaybackTime - Notify.TriggerTime <= Notify.Duration)
                    {
                        SkeletalMeshComponent->TickAnimNotify(Notify, DeltaTime);
                    }
                    else if(PlaybackContext->PreviousTime <= EndTime && PlaybackContext->PlaybackTime >= EndTime)
                    {
                        SkeletalMeshComponent->EndAnimNotify(Notify);
                    }
                }
            }
            //역방향 재생시 반대로 계산
            else if (PlaybackContext->PlayRate < 0)
            {
                if (PlaybackContext->PreviousTime >= Notify.TriggerTime && PlaybackContext->PlaybackTime <= Notify.TriggerTime)
                {
                    SkeletalMeshComponent->StartAnimNotify(Notify);
                }
                else if (PlaybackContext->PreviousTime < Notify.TriggerTime && Notify.Duration > 0)
                {
                    float EndTime = Notify.TriggerTime + Notify.Duration;
                    if (Notify.TriggerTime - PlaybackContext->PlaybackTime <= Notify.Duration)
                    {
                        SkeletalMeshComponent->TickAnimNotify(Notify, DeltaTime);
                    }
                    else if (PlaybackContext->PreviousTime >= EndTime && PlaybackContext->PlaybackTime <= EndTime)
                    {
                        SkeletalMeshComponent->EndAnimNotify(Notify);
                    }
                }
            }
        }
    }
}

void UAnimInstance::NativeUpdateAnimation(float DeltaTime)
{

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

FAnimationPlaybackContext::FAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
    : AnimationAsset(InAnimAsset), bIsLooping(IsLoop), PlayRate(InPlayRate), StartPosition(InStartPosition)
{
    PreviousTime = 0.f;
    PlaybackTime = 0.f;
    AnimationLength = Cast<UAnimSequence>(InAnimAsset)->GetPlayLength();
}
