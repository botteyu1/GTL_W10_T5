#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class USkeleton;
class UAnimationAsset : public UObject
{
    DECLARE_CLASS(UAnimationAsset, UObject)
public:
    UAnimationAsset() = default;
    virtual ~UAnimationAsset() override = default;

public:
    void SetSkeleton(USkeleton* Skeleton);
protected:
    USkeleton* Skeleton;
    //TArray<TObjectPtr<UAnimMetaData>> MetaData;
};
