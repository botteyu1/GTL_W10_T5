#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimInstance.h"

class UAnimationStateMachine;

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

// 1:N 상태 머신 기반 애니메이션
class UFiniteStateMachineAnimInstance : public UAnimInstance
{
    DECLARE_CLASS(UFiniteStateMachineAnimInstance, UAnimInstance);
public:
    UFiniteStateMachineAnimInstance();
    virtual ~UFiniteStateMachineAnimInstance();

    // 상태 머신 초기화
    // StateMachine 객체를 생성하고 델리게이트를 연결(Bind)
    virtual void Initialize(USkeletalMeshComponent* MeshComponent) override;

    // 매 틱(프레임) 상태 머신 업데이트
    virtual void TriggerAnimNotifies(float DeltaTime) override;

    // ③ 입력에 따른 상태 강제 변경 API
    void OnInputQ();  // Q 입력 시 Idle
    void OnInputW();  // W 입력 시 Walk
    void OnInputE();  // E 입력 시 Run
    void OnInputR();  // R 입력 시 Fly

private:
    // 상태 결정 로직 담당하는 객체
    UAnimationStateMachine* StateMachine;
    // 상태 변화시 전달받을 델리게이트
    void OnStateChanged(EAnimState Previous, EAnimState Current);
};
