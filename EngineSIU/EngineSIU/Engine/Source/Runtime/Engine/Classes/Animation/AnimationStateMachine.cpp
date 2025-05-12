#include "AnimationStateMachine.h"
#include "FiniteStateMachineAnimInstance.h"

UAnimationStateMachine::UAnimationStateMachine()
    : Owner(nullptr), CurrentState(EAnimState::Idle)
{
}

UAnimationStateMachine::~UAnimationStateMachine()
{
}

void UAnimationStateMachine::Initialize(UFiniteStateMachineAnimInstance* OwnerInstance, const FOnAnimStateChanged& Delegate)
{
    Owner = OwnerInstance;
    CurrentState = EAnimState::Idle;
    OnStateChanged = Delegate;
}

void UAnimationStateMachine::ProcessState(float DeltaTime)
{
    float speed = 0.f;

    //if (Owner && Owner->GetSkeletalMeshComponent())
    //    speed = Owner->GetSkeletalMeshComponent()->GetVelocity().Size();

    EAnimState NewState = CurrentState;
    const float IdleThreshold = 1e-3f;
    const float WalkThreshold = 100.0f;
    const float RunThreshold = 300.0f;

    if (speed < IdleThreshold)
    {
        NewState = EAnimState::Idle;
    }
    else if (speed < WalkThreshold)
    {
        NewState = EAnimState::Walk;
    }
    else if (speed < RunThreshold)
    {
        NewState = EAnimState::Run;
    }
    else
    {
        NewState = EAnimState::Fly;

    if (NewState != CurrentState)
    {
        ForceState(NewState);

        EAnimState Previous = CurrentState;
        CurrentState = NewState;
        if (OnStateChanged.IsBound())
            OnStateChanged.Execute(Previous, NewState);
}


