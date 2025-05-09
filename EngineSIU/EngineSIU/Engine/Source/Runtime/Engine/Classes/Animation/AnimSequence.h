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

    // TODO: 맵의 key는 int가 아니라 fname이어야 함.
    TArray<TMap<int32, FTransform>> Anim;

    int32 FrameRate = 30;

    int32 NumFrames = 240;
};
