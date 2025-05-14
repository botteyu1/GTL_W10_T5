#include "Character.h"
#include "Animation/AnimDataModel.h"
#include "LuaScripts/AnimFSMLuaScriptComponent.h"

ACharacter::ACharacter()
{
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    LuaScriptComponent = AddComponent<UAnimFSMLuaScriptComponent>("UAnimFSMLuaScriptComponent");
}

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent& Notify, ENotifyState NotifyState, float DeltaTime)
{
    if (ULuaScriptComponent* LuaComp = GetComponentByClass<ULuaScriptComponent>())
    {
        switch (NotifyState)
        {
        case ENotifyState::Start:
            LuaComp->CallLuaFunction("StartAnimNotify", *Notify.NotifyName.ToString());
            break;
        case ENotifyState::Tick:
            LuaComp->CallLuaFunction("TickAnimNotify", *Notify.NotifyName.ToString(), DeltaTime);
            break;
        case ENotifyState::End:
            LuaComp->CallLuaFunction("EndAnimNotify", *Notify.NotifyName.ToString());
            break;
        }
    }
}
