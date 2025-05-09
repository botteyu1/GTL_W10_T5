#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "AnimRawData.h"

struct FBoneAnimationTrack
{
    FName Name;
    FRawAnimSequenceTrack InternalTrack;
};

struct FFrameRate
{
    double FrameRate;
};

struct FNamedFloatCurve
{
    FName CurveName;
    TArray<FFloatKey> Keys;

    FNamedFloatCurve(const FName& InName = NAME_None) : CurveName(InName) {}
};

// 스켈레탈 본의 움직임을 제외한 부가적인 애니메이션 커브 데이터를 저장하는 구조체
struct FAnimationCurveData
{
    FName CurveName;
    TArray<FNamedFloatCurve> FloatCurves;

    FAnimationCurveData(const FName& InName = "")
        : CurveName(InName)
    {
    }
};


class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject)
public:
    UAnimDataModel();
    virtual ~UAnimDataModel() = default;

    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;

    virtual const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;
};
