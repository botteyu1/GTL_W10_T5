#include "Character.h"
#include "Animation/AnimDataModel.h"

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent& Notify)
{
    if (Notify.NotifyName == FName(TEXT("Jump")))
    {
        AddActorLocation(FVector(0, 0, 10));
    }
}
