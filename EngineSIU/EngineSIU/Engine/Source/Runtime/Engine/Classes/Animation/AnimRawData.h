#pragma once
#include "Container/Array.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

/**
 * 특정 시간에 대한 FVector 값을 가지는 키프레임입니다.
 */
struct FVectorKey
{
    float Time;     // 키프레임의 시간 (초)
    FVector Value;  // 해당 시간에서의 위치 또는 스케일 값

    FVectorKey(float InTime = 0.0f, const FVector& InValue = FVector::ZeroVector)
        : Time(InTime), Value(InValue)
    {
    }
};

/**
 * 특정 시간에 대한 FQuat 값을 가지는 키프레임입니다.
 */
struct FQuatKey
{
    float Time;     // 키프레임의 시간 (초)
    FQuat Value;    // 해당 시간에서의 회전 값

    FQuatKey(float InTime = 0.0f, const FQuat& InValue = FQuat::Identity)
        : Time(InTime), Value(InValue)
    {
    }
};

/**
 * 특정 시간에 대한 float 값을 가지는 키프레임입니다.
 * FAnimationCurveData 내의 커브에 사용됩니다.
 */
struct FFloatKey
{
    float Time;     // 키프레임의 시간 (초)
    float Value;    // 해당 시간에서의 float 값

    // 필요에 따라 보간 정보(탄젠트 등)를 추가할 수 있습니다.
    // EInterpolationMode InterpMode;
    // float ArriveTangent;
    // float LeaveTangent;

    FFloatKey(float InTime = 0.0f, float InValue = 0.0f)
        : Time(InTime), Value(InValue)
    {
    }
};


struct FRawAnimSequenceTrack
{
    TArray<FVectorKey> PosKeys;
    TArray<FQuatKey> RotKeys;
    TArray<FVectorKey> ScaleKeys;

    bool IsEmpty() const
    {
        return PosKeys.Num() == 0 && RotKeys.Num() == 0 && ScaleKeys.Num() == 0;
    }
};

