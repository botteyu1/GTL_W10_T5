#pragma once

#include "Player.h"

class ACharacter : public APlayer
{
public :
    virtual void HandleAnimNotify(const FAnimNotifyEvent& Notify) override;
};
