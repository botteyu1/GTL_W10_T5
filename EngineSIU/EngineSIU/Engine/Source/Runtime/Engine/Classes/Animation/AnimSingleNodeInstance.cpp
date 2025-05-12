#include "AnimSingleNodeInstance.h"

UAnimSingleNodeInstance::UAnimSingleNodeInstance()
{
}

UAnimSingleNodeInstance::~UAnimSingleNodeInstance()
{
}

void UAnimSingleNodeInstance::NativeUpdateAnimation(float DeltaTime)
{

}

void UAnimSingleNodeInstance::SetAnimationAsset(UAnimationAsset* InAnimationAsset)
{
   
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
