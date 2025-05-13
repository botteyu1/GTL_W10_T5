#pragma once

#include "AnimInstance.h"

class UAnimationAsset;

class UAnimSingleNodeInstance : public UAnimInstance
{
    DECLARE_CLASS(UAnimSingleNodeInstance, UAnimInstance);
public:
    UAnimSingleNodeInstance();
    virtual ~UAnimSingleNodeInstance() override;
    UObject* Duplicate(UObject* InOuter) override;
    

    virtual void NativeUpdateAnimation(float DeltaTime) override;
    void UpdateBone(float ElapsedTime);
    void SetAnimationAsset(UAnimationAsset* InAnimAsset, bool IsLoop = false, float InPlayRate = 1.f, float InStartPosition = 0.f);

    UAnimationAsset* GetAnimationAsset() const { return CurrentAsset; }

    void PlayAnim(bool bIsLooping = false, float InPlayRate = 1.f, float InStartPosition = 0.f);
    void StopAnim();

    void SetPlaying(bool bInPlaying);
    bool IsPlaying() const;
    void PauseAnim();
    void SetLooping(bool bInLooping);
    bool IsLooping();

    void SetAnimationTime(float InTime);


    virtual void AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop = false, float InPlayRate = 1.f, float InStartPosition = 0.f) override;
protected:
    UAnimationAsset* CurrentAsset = nullptr;
    bool bIsPlaying = false;
};
