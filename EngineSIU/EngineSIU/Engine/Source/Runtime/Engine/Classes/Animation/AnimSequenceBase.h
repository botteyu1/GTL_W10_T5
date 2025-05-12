#pragma once
#include "AnimDataModel.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Asset/AnimationAsset.h"

class UAnimDataModel;
struct FBoneAnimationTrack;

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


protected:
    UAnimDataModel* AnimDataModel = nullptr;
    // !TODO : AnimNofity
    //TArray<FAnimNofity*> Notifies;
};
