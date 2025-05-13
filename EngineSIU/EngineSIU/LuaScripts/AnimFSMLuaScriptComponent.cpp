#include "AnimFSMLuaScriptComponent.h"


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
    LuaState.set_function("changeAnimState", [&](const std::string& PrevStateName, const std::string& CurrStateName, float BlendTime)
        {
            ChangeAnimState_LuaImpl(PrevStateName.c_str(), CurrStateName.c_str(), BlendTime);
        }
    );
}

void UAnimFSMLuaScriptComponent::AddAnimState_LuaImpl(const FString& StateName, const FString& AnimName)
{
    
}

void UAnimFSMLuaScriptComponent::RemoveAnimState_LuaImpl(const FString& StateName)
{
}

void UAnimFSMLuaScriptComponent::ChangeAnimState_LuaImpl(const FString& PrevStateName, const FString& CurrStateName, float BlendTime)
{
}
