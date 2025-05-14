#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimSequenceBase.h"

struct FTransform;

// TODO: 임시로 만든 클래스
class UAnimSequence : public UAnimSequenceBase
{
    DECLARE_CLASS(UAnimSequence, UObject)

public:
    UAnimSequence();
    virtual ~UAnimSequence() override = default;

    FName GetName() const;
};
