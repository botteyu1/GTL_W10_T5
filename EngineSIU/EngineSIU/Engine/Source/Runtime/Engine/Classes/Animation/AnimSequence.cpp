
#include "AnimSequence.h"

#include "Math/Transform.h"
#include "Animation/AnimDataModel.h"

UAnimSequence::UAnimSequence()
{
    for (int32 i = 0; i < NumFrames; ++i)
    {
        TMap<int32, FTransform> Track;
        
        for (int32 j = 8; j < 10; ++j)
        {
            FTransform TF;
            TF.Rotation = FRotator(
                    0.f,
                    0.f,
                    (FMath::Sin((static_cast<float>(i) / FrameRate) * PI * 1.f) - 1.f) * 30.f
                ).Quaternion();
            Track.Add(j, TF);
        }
        Track[8].Rotation = FRotator(-70.f, 0.f, 30.f).Quaternion() * Track[8].Rotation;
        
        for (int32 j = 32; j < 34; ++j)
        {
            FTransform TF;
            TF.Rotation = FRotator(
                    0.f,
                    0.f,
                    (FMath::Sin((static_cast<float>(i) / FrameRate) * PI * 1.f) + 1.f) * 30.f
                ).Quaternion();
            Track.Add(j, TF);
        }
        Track[32].Rotation = FRotator(-70.f, 00.f, -30.f).Quaternion() * Track[32].Rotation;
        
        for (int32 j = 60; j < 62; ++j)
        {
            FTransform TF;
            TF.Rotation = FRotator(
                    (FMath::Sin((static_cast<float>(i) / FrameRate) * PI * 1.f) + 1.0f) * 30.f,
                    0.f,
                    0.f
                ).Quaternion();
            Track.Add(j, TF);
        }
        Track[60].Rotation = FRotator(-30.f, 0.f, 0.f).Quaternion() * Track[60].Rotation;
        
        for (int32 j = 55; j < 57; ++j)
        {
            FTransform TF;
            TF.Rotation = FRotator(
                    (FMath::Sin((static_cast<float>(i) / FrameRate) * PI * 1.f) - 1.0f) * -30.f,
                    0.f,
                    0.f
                ).Quaternion();
            Track.Add(j, TF);
        }
        Track[55].Rotation = FRotator(-30.f, 0.f, 0.f).Quaternion() * Track[55].Rotation;

        FTransform TF;
        TF.Translation = FVector(
                0.f,
                0.f,
                (FMath::Sin((static_cast<float>(i) / FrameRate) * PI * 2.f + 1.f) + 1.0f) * 2.f
            );
        Track.Add(0, TF);

        Anim.Add(Track);
    }
}

FName UAnimSequence::GetName() const
{
    return AnimDataModel ? AnimDataModel->Name : NAME_None;
}

