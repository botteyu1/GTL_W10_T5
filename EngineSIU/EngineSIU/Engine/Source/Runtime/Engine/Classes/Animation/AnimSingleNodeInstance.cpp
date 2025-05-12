#include "AnimSingleNodeInstance.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{
}

UAnimSingleNodeInstance::~UAnimSingleNodeInstance()
{
}

void UAnimSingleNodeInstance::UpdateAnimation(float DeltaTime)
{
}

void UAnimSingleNodeInstance::SetAnimationAsset(UAnimationAsset* InAnimationAsset)
{
}

void UAnimSingleNodeInstance::PlayAnim(bool bIsLooping, float InPlayRate, float InStartPosition)
{
}

void UAnimSingleNodeInstance::StopAnim()
{
}

void UAnimSingleNodeInstance::SetPlaying(bool bInPlaying)
{
}

bool UAnimSingleNodeInstance::IsPlaying() const
{
    return false;
}

void UAnimSingleNodeInstance::SetLooping(bool bInLooping)
{
}

bool UAnimSingleNodeInstance::IsLooping() const
{
    return false;
}
