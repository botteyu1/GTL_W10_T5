#pragma once
#include "LuaScriptComponent.h"

struct FString;
class UFSMAnimInstance;

class UAnimFSMLuaScriptComponent : public ULuaScriptComponent
{
    DECLARE_CLASS(UAnimFSMLuaScriptComponent, ULuaScriptComponent)
public:
    UAnimFSMLuaScriptComponent();
    virtual ~UAnimFSMLuaScriptComponent() override;
protected:
    virtual void BindEngineAPI() override;

private:
    void AddAnimState_LuaImpl(const FString& StateName, const FString& AnimName);
    void RemoveAnimState_LuaImpl(const FString& StateName);
    void ChangeAnimState_LuaImpl(const FString& PrevStateName, const FString& CurrStateName, float BlendTime);

    UFSMAnimInstance* AnimInstance = nullptr;
};
