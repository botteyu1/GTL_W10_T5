#include "Character.h"
#include "Animation/AnimDataModel.h"
#include <LuaScripts/LuaScriptComponent.h>

void ACharacter::StartAnimNotify(const FAnimNotifyEvent& Notify)
{
    if (ULuaScriptComponent* LuaComp = GetComponentByClass<ULuaScriptComponent>())
    {
        LuaComp->CallLuaFunction("StartAnimNotify", *Notify.NotifyName.ToString());
    }
}

void ACharacter::TickAnimNotify(const FAnimNotifyEvent& Notify, float DeltaTime)
{
    if (ULuaScriptComponent* LuaComp = GetComponentByClass<ULuaScriptComponent>())
    {
        LuaComp->CallLuaFunction("TickAnimNotify", *Notify.NotifyName.ToString(), DeltaTime);
    }
}

void ACharacter::EndAnimNotify(const FAnimNotifyEvent& Notify)
{
    if (ULuaScriptComponent* LuaComp = GetComponentByClass<ULuaScriptComponent>())
    {
        LuaComp->CallLuaFunction("EndAnimNotify", *Notify.NotifyName.ToString());
    }
}
