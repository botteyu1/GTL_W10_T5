#pragma once

#include "AnimInstance.h"

class UAnimationAsset;

class UAnimSingleNodeInstance : public UAnimInstance
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UAnimInstance);
public:
    UAnimSingleNodeInstance();
    virtual ~UAnimSingleNodeInstance() override;

    void UpdateAnimation(float DeltaTime);
    void SetAnimationAsset(UAnimationAsset* InAnimationAsset);

    UAnimationAsset* GetAnimationAsset() const { return CurrentAsset; }

    void PlayAnim(bool bIsLooping = false, float InPlayRate = 1.f, float InStartPosition = 0.f);
    void StopAnim();

    void SetPlaying(bool bInPlaying);
    bool IsPlaying() const;
    void SetLooping(bool bInLooping);
    bool IsLooping() const;


protected:
    UAnimationAsset* CurrentAsset = nullptr;
};
