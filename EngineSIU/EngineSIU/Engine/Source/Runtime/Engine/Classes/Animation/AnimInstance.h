#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject);
public:
    UAnimInstance();
    virtual ~UAnimInstance();
    // 애니메이션 업데이트 함수
    virtual void NativeUpdateAnimation(float DeltaTime);
    // 애니메이션 블렌딩 함수
    virtual void NativeBlendAnimation(float DeltaTime);
    // 애니메이션 재생 함수
    virtual void NativePlayAnimation(float DeltaTime);
};
