#pragma once
#include "AnimDataModel.h"
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

    FFrameRate GetFrameRate() const;

    const TArray<FBoneAnimationTrack>* GetBoneAnimationTracks() const;

    void AddAnimNotifyEvent(FAnimNotifyEvent Notify);
    void RemoveAnimNotifyEvent(FAnimNotifyEvent Notify);
    void RemoveAnimNotifyEvent(int index);
    void ClearAnimNotifyEvents();
    void SortNotifyEvents();
    TArray<FAnimNotifyEvent>& GetAnimNotifies(){ return Notifies; }


protected:
    UAnimDataModel* AnimDataModel = nullptr;
    // !TODO : AnimNotify
    TArray<FAnimNotifyEvent> Notifies;
};
