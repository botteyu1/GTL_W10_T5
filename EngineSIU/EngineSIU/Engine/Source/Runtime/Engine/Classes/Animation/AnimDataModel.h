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
    uint32 Numerator;
    uint32 Denominator;

    bool IsValid() const
    {
        return Numerator > 0;
    }

    double AsInterval() const
    {
        return double(Denominator) / double(Numerator);
    }

    double AsDecimal() const
    {
        return double(Numerator) / double(Denominator);
    }
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

class USkeleton;

class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject)
public:
    UAnimDataModel();
    virtual ~UAnimDataModel() = default;

    FName Name;
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    FFrameRate FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    FAnimationCurveData CurveData;


    // !TODO : 이거 어떻게 사용해야 하는지 고민
    USkeleton* TargetSkeleton;

    virtual const TArray<FBoneAnimationTrack>& GetBoneAnimationTracks() const;

};
