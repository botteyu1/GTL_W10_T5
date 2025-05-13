#pragma once

#include "Player.h"
#include "UObject/ObjectMacros.h"

class ACharacter : public APlayer
{
    DECLARE_CLASS(ACharacter, APlayer)
public :
    ACharacter() = default;
    virtual ~ACharacter() override = default;
    virtual void StartAnimNotify(const FAnimNotifyEvent& Notify) override;

    virtual void TickAnimNotify(const FAnimNotifyEvent& Notify, float DeltaTime = 0.f);

    virtual void EndAnimNotify(const FAnimNotifyEvent& Notify);
};
