#pragma once
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


protected:
    UAnimDataModel* AnimDataModel = nullptr;
    // !TODO : AnimNofity
    //TArray<FAnimNofity*> Notifies;
};
