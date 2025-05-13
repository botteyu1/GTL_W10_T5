#pragma once
#include "AnimInstance.h"

class UAnimSequence;
struct FTransform;

// 블렌딩 테스트용 애님 인스턴스
class UBlendAnimInstance : public UAnimInstance
{
    DECLARE_CLASS(UBlendAnimInstance, UAnimInstance);
public:
    UBlendAnimInstance();
    virtual ~UBlendAnimInstance() override;

    UObject* Duplicate(UObject* InOuter) override;

    virtual void NativeUpdateAnimation(float DeitaTime) override;
    void SetAnimSequences(UAnimSequence* InAnimA, UAnimSequence* InAnimB);

    FTransform GetCurrentTransform(UAnimSequence* AnimInstance, FName BoneName);
    void SetAnimationEnabled(bool bEnable)
    {
        bIsPlaying = bEnable;
    }

public:
    float BlendAlpha = 0.0f;

public:
    UAnimSequence* AnimA = nullptr;
    UAnimSequence* AnimB = nullptr;

protected:
    bool bIsPlaying = false;
};
