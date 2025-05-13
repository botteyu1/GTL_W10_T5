#include "AnimFSMLuaScriptComponent.h"
#include "Animation/FSMAnimInstance.h"

UAnimFSMLuaScriptComponent::UAnimFSMLuaScriptComponent()
{
}

UAnimFSMLuaScriptComponent::~UAnimFSMLuaScriptComponent()
{
}

void UAnimFSMLuaScriptComponent::BindEngineAPI()
{
    Super::BindEngineAPI();
    // FSM 관련 API 바인딩
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

void UAnimFSMLuaScriptComponent::AddAnimState_LuaImpl(const FString& StateName, const FString& AnimName)
{
    if (AnimInstance)
    {
        AnimInstance->AddAnimState(StateName, AnimName);
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("AnimInstance is not valid"));
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
        UE_LOG(ELogLevel::Error, TEXT("AnimInstance is not valid"));
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
        UE_LOG(ELogLevel::Error, TEXT("AnimInstance is not valid"));
    }
}
