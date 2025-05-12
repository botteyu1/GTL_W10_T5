#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class USkeletalMeshComponent;

// 애니메이션 상태를 나타내는 enum
enum class EAnimState
{
    Idle,
    Walk,
    Run,
    Fly
};

// Delegate 타입 선언 (이전 상태, 현재 상태 전달)
#include "Delegates/Delegate.h"
DECLARE_DELEGATE_TwoParams(FOnAnimStateChanged, EAnimState /*Previous*/, EAnimState /*Current*/);

// UAnimInstance: 추상 베이스 클래스
class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject);
public:
    UAnimInstance();
    virtual ~UAnimInstance();

    // 애니메이션 초기화 (컴포넌트 등 설정)
    virtual void Initialize(USkeletalMeshComponent* MeshComponent) = 0;

    // 매 프레임 애니메이션 업데이트 함수
    virtual void TriggerAnimNotifies(float DeltaTime) = 0;

    // FSM 이 메시 컴포넌트에 접근할 수 있도록 getter 제공
    USkeletalMeshComponent* GetSkeletalMeshComponent() const
    {
        return SkeletalMeshComponent;
    }

protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
};
