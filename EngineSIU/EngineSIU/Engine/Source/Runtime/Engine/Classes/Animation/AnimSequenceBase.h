#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Asset/AnimationAsset.h"

class UAnimDataModel;
class UAnimSequenceBase : public UAnimationAsset
{
    DECLARE_CLASS(UAnimSequenceBase, UAnimationAsset)
public:
    UAnimSequenceBase();
    virtual ~UAnimSequenceBase() override = default;

public:
    UAnimDataModel* GetAnimDataModel() const;
    void SetAnimDataModel(UAnimDataModel* InAnimDataModel);
protected:
    UAnimDataModel* AnimDataModel;
    // !TODO : AnimNofity
    //TArray<FAnimNofity*> Notifies;
};
