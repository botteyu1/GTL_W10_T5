#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Asset/AnimationAsset.h"

class UAnimDataModel;
struct FBoneAnimationTrack;
struct FAnimNotifyEvent;

class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)
public:
    UAnimSequenceBase();
    virtual ~UAnimSequenceBase() override = default;

public:
    UAnimDataModel* GetAnimDataModel() const;
    void SetAnimDataModel(UAnimDataModel* InAnimDataModel);
    float GetPlayLength() const;

    void AddAnimNotifyEvent(FAnimNotifyEvent Notify);
    void RemoveAnimNotifyEvent(FAnimNotifyEvent Notify);
    void ClearAnimNotifyEvents();
    void SortNotifyEvents();
    const TArray<FAnimNotifyEvent>& GetAnimNotifies() const { return Notifies; }


protected:
    UAnimDataModel* AnimDataModel = nullptr;
    // !TODO : AnimNotify
    TArray<FAnimNotifyEvent> Notifies;
};
