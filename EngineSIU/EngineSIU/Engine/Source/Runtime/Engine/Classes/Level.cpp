#include "Level.h"
#include "GameFramework/Actor.h"
#include "UObject/Casts.h"
#include "Engine/Engine.h"
#include "Engine/World/World.h"


void ULevel::InitLevel(UWorld* InOwningWorld)
{
    OwningWorld = InOwningWorld;
    //Notify Lua 테스트용 임시 코드
    if (OwningWorld->WorldType == EWorldType::Editor)
    {
        if (!OwningWorld->GetPlayerController())
        {
            APlayerController* PlayerController = OwningWorld->SpawnActor<APlayerController>();
            PlayerController->SetActorLabel(TEXT("OBJ_PLAYER_CONTROLLER"));
            PlayerController->SetActorTickInEditor(false);
            OwningWorld->SetPlayerController(PlayerController);
        }
    }
}

void ULevel::Release()
{
    
    
    for (AActor* Actor : Actors)
    {
        Actor->EndPlay(EEndPlayReason::WorldTransition);
        TSet<UActorComponent*> Components = Actor->GetComponents();
        for (UActorComponent* Component : Components)
        {
            GUObjectArray.MarkRemoveObject(Component);
        }
        GUObjectArray.MarkRemoveObject(Actor);
    }
    Actors.Empty();
}

UObject* ULevel::Duplicate(UObject* InOuter)
{
    ThisClass* NewLevel = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewLevel->OwningWorld = OwningWorld;

    for (AActor* Actor : Actors)
    {
        NewLevel->Actors.Emplace(static_cast<AActor*>(Actor->Duplicate(InOuter)));
    }

    return NewLevel;
}
