#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class USkeletalMeshComponent;

class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject);
public:
    UAnimInstance();
    virtual ~UAnimInstance();
    // 애니메이션 업데이트 함수
    void TriggerAnimNotifies(float DeltaTime);

    void Initialize(USkeletalMeshComponent* MeshComponent);

protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
};
