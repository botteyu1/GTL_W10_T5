#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimInstance.h"
#include "FiniteStateMachineAnimInstance.h"
#include "Container/Map.h"
class UFiniteStateMachineAnimInstance;

class UAnimationStateMachine : public UObject
{
    DECLARE_CLASS(UAnimationStateMachine, UObject);

public:
    UAnimationStateMachine();
    virtual ~UAnimationStateMachine();

    // 소유 인스턴스 및 델리게이트 바인딩
    void Initialize(UFiniteStateMachineAnimInstance* OwnerInstance, const FOnAnimStateChanged& Delegate);

    // 상태 갱신
    void ProcessState(float DeltaTime);

    // 입력 등에 의해 바로 상태 전활을 원할 때 호출
    void ForceState(EAnimState NewState);

    void AddState(EAnimState State /*Lambda*/);
private:
    EAnimState CurrentState = EAnimState::Idle;
    FOnAnimStateChanged OnStateChanged;
    //TMap<EAnimState, std::function> StateMap;
    //TMAp<TPair<State,State>, Lamda>;
};



