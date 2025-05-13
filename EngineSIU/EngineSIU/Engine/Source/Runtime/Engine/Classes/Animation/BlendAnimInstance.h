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

    virtual void NativeUpdateAnimation(float DeitaTime) override;
    void SetAnimSequences(UAnimSequence* InAnimA, UAnimSequence* InAnimB);
    void SetAnimationEnabled(bool bEnable);

public:
    float BlendAlpha = 0.0f;

public:
    UAnimSequence* AnimA = nullptr;
    UAnimSequence* AnimB = nullptr;

protected:
    bool bIsPlaying = false;
};
