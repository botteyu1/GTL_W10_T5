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
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
    virtual void BindEngineAPI() override;

private:
    void StartAnim_LuaImpl();
    void StopAnim_LuaImpl();
    void AddAnimState_LuaImpl(const FString& StateName, const FString& AnimName);
    void RemoveAnimState_LuaImpl(const FString& StateName);
    void ChangeAnimState_LuaImpl(const FString& NewStateName, float BlendTime, bool bLooping);

    UFSMAnimInstance* AnimInstance = nullptr;
};
