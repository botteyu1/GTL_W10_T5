#include "Character.h"
#include "Animation/AnimDataModel.h"

void ACharacter::StartAnimNotify(const FAnimNotifyEvent& Notify)
{
    if (Notify.NotifyName == FName(TEXT("Up")))
    {
        UE_LOG(ELogLevel::Display, "Start Notify: %s", *Notify.NotifyName.ToString());
    }
}

void ACharacter::TickAnimNotify(const FAnimNotifyEvent& Notify, float DeltaTime)
{
    if (Notify.NotifyName == FName(TEXT("Up")))
    {
        AddActorLocation(FVector(0, 0, 0.2f));
    }
}

void ACharacter::EndAnimNotify(const FAnimNotifyEvent& Notify)
{
    if (Notify.NotifyName == FName(TEXT("Up")))
    {
        SetActorLocation(FVector(0));
    }
}
