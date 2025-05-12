#include "Character.h"
#include "Animation/AnimDataModel.h"

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent& Notify)
{
    if (Notify.NotifyName == FName(TEXT("Fire")))
    {
        //Fire();
    }
}
