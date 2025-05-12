#include "AnimInstance.h"
#include "Engine/Asset/AnimationAsset.h"
#include "Animation/AnimSequenceBase.h"
#include "UObject/Casts.h"
#include "Components/SkeletalMeshComponent.h"

UAnimInstance::UAnimInstance()
{
}

UAnimInstance::~UAnimInstance()
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaTime)
{
    for (const auto& PlaybackContexts : AnimationPlaybackContexts)
    {
        UAnimSequenceBase* AnimSequenceBase = Cast<UAnimSequenceBase>(PlaybackContexts->AnimationAsset);
        for (const auto& Notify : AnimSequenceBase->GetAnimNotifies())
        {
            if (PlaybackContexts->PreviousTime <= Notify.TriggerTime && PlaybackContexts->PlaybackTime >= Notify.TriggerTime)
            {
                SkeletalMeshComponent->HandleAnimNotify(Notify);
            }
        }
    }
}

void UAnimInstance::AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
{
    if (InAnimAsset)
    {
        auto PlaybackContext = std::make_shared<FAnimationPlaybackContext>(InAnimAsset, IsLoop, InPlayRate, InStartPosition);
        AnimationPlaybackContexts.Add(PlaybackContext);
    }
}

std::shared_ptr<FAnimationPlaybackContext>& UAnimInstance::GetAnimationPlaybackContext(UAnimationAsset* InAnimAsset)
{
    for (auto& PlaybackContext : AnimationPlaybackContexts)
    {
        if (PlaybackContext->AnimationAsset == InAnimAsset)
            return PlaybackContext;
    }
}

void UAnimInstance::Initialize(USkeletalMeshComponent* MeshComponent)
{
    SkeletalMeshComponent = MeshComponent;
    AnimationPlaybackContexts.Empty();
}

FAnimationPlaybackContext::FAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop, float InPlayRate, float InStartPosition)
    : AnimationAsset(InAnimAsset), bIsLooping(IsLoop), PlayRate(InPlayRate), StartPosition(InStartPosition)
{
    PreviousTime = 0.f;
    PlaybackTime = 0.f;
}
