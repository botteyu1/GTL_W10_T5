#include "FiniteStateMachineAnimInstance.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectFactory.h"
#include "AnimationStateMachine.h"
UFiniteStateMachineAnimInstance::UFiniteStateMachineAnimInstance()
{
}

UFiniteStateMachineAnimInstance::~UFiniteStateMachineAnimInstance()
{
}

void UFiniteStateMachineAnimInstance::Initialize(USkeletalMeshComponent* MeshComponent)
{
    // 부모 클래스 초기화
    Super::Initialize(MeshComponent);

        // StateMachine 생성 및 델리게이트 연결(Bind)
        if (!StateMachine)
        {
            StateMachine = FObjectFactory::ConstructObject<UAnimationStateMachine>(this);
        }
        //StateMachine->AddState(EAnimState::Idle, [this]() { NativeUpdateAnimation(IdleAnimation); });
        //StateMachine->AddState(EAnimState::Walk, [this]() { NativeUpdateAnimation(WalkAnimation); });
        
}

void UFiniteStateMachineAnimInstance::TriggerAnimNotifies(float DeltaTime)
{
}

void UFiniteStateMachineAnimInstance::OnStateChanged(EAnimState Previous, EAnimState Current)
{
}
