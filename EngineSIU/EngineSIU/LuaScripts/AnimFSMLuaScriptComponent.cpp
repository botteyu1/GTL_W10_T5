#include "AnimFSMLuaScriptComponent.h"
#include "Animation/FSMAnimInstance.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"

UAnimFSMLuaScriptComponent::UAnimFSMLuaScriptComponent()
{
}

UAnimFSMLuaScriptComponent::~UAnimFSMLuaScriptComponent()
{
}

void UAnimFSMLuaScriptComponent::BeginPlay()
{
    if (GetOwner() && GetOwner()->GetComponentByClass<USkeletalMeshComponent>())
    {
        USkeletalMeshComponent* SkelComp = GetOwner()->GetComponentByClass<USkeletalMeshComponent>();
        if (SkelComp && Cast<UFSMAnimInstance>(SkelComp->GetAnimInstance()))
        {
            AnimInstance = Cast<UFSMAnimInstance>(SkelComp->GetAnimInstance());
        }
    }
    Super::BeginPlay();
}

void UAnimFSMLuaScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AnimInstance)
    {
        // !TODO : 클리어 로직
    }
    AnimInstance = nullptr;
    Super::EndPlay(EndPlayReason);
}

void UAnimFSMLuaScriptComponent::BindEngineAPI()
{
    Super::BindEngineAPI();
    // FSM 관련 API 바인딩
    LuaState.set_function("startAnim", [&]()
        {
            StartAnim_LuaImpl();
        }
    );
    LuaState.set_function("stopAnim", [&]()
        {
            StopAnim_LuaImpl();
        }
    );
    LuaState.set_function("addAnimState", [&](const std::string& StateName, const std::string& AnimName)
        {
            AddAnimState_LuaImpl(StateName.c_str(), AnimName.c_str());
        }
    );
    LuaState.set_function("removeAnimState", [&](const std::string& StateName)
        {
            RemoveAnimState_LuaImpl(StateName.c_str());
        }
    );
    LuaState.set_function("changeAnimState", [&](const std::string& NewStateName, float BlendTime, bool bLooping)
        {
            ChangeAnimState_LuaImpl(NewStateName.c_str(), BlendTime, bLooping);
        }
    );
}

void UAnimFSMLuaScriptComponent::StartAnim_LuaImpl()
{
    if (AnimInstance)
    {
        AnimInstance->SetPlaying(true);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("UAnimFSMLuaScriptComponent::StartAnim_LuaImpl : AnimInstance is not valid"));
    }
}

void UAnimFSMLuaScriptComponent::StopAnim_LuaImpl()
{
    if (AnimInstance)
    {
        AnimInstance->SetPlaying(false);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("UAnimFSMLuaScriptComponent::StopAnim_LuaImpl : AnimInstance is not valid"));
    }
}

void UAnimFSMLuaScriptComponent::AddAnimState_LuaImpl(const FString& StateName, const FString& AnimName)
{
    if (AnimInstance)
    {
        AnimInstance->AddAnimState(StateName, AnimName);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("UAnimFSMLuaScriptComponent::AddAnimState_LuaImpl : AnimInstance is not valid"));
    }
}

void UAnimFSMLuaScriptComponent::RemoveAnimState_LuaImpl(const FString& StateName)
{
    if (AnimInstance)
    {
        AnimInstance->RemoveAnimState(StateName);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("UAnimFSMLuaScriptComponent::RemoveAnimState_LuaImpl :  AnimInstance is not valid"));
    }
}

void UAnimFSMLuaScriptComponent::ChangeAnimState_LuaImpl(const FString& NewStateName, float BlendTime, bool bLooping)
{
    if (AnimInstance)
    {
        AnimInstance->ChangeAnimState(NewStateName, BlendTime, bLooping);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("UAnimFSMLuaScriptComponent::ChangeAnimState_LuaImpl : AnimInstance is not valid"));
    }
}
