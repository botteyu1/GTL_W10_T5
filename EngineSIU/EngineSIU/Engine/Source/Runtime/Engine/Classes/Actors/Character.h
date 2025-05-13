#pragma once

#include "Player.h"
#include "UObject/ObjectMacros.h"

class ACharacter : public APlayer
{
    DECLARE_CLASS(ACharacter, APlayer)
public :
    ACharacter() = default;
    virtual ~ACharacter() override = default;
    virtual void HandleAnimNotify(const FAnimNotifyEvent& Notify) override;
};
