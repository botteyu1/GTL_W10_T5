#pragma once

#include "Player.h"
#include "UObject/ObjectMacros.h"

class UAnimFSMLuaScriptComponent;

class ACharacter : public APlayer
{
    DECLARE_CLASS(ACharacter, APlayer)
public :
    ACharacter();
    virtual ~ACharacter() override = default;
    virtual void PostSpawnInitialize() override;

    virtual void HandleAnimNotify(const FAnimNotifyEvent& Notify, ENotifyState NotifyState, float DeltaTime = 0.f) override;

private:
    UAnimFSMLuaScriptComponent* LuaScriptComponent = nullptr;
};
