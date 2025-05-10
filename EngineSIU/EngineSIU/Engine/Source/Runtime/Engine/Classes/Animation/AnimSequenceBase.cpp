#include "AnimSequenceBase.h"
#include "Animation/AnimDataModel.h"

UAnimSequenceBase::UAnimSequenceBase()
{
}

UAnimDataModel* UAnimSequenceBase::GetAnimDataModel() const
{
    return AnimDataModel;
}

void UAnimSequenceBase::SetAnimDataModel(UAnimDataModel* InAnimDataModel)
{
    AnimDataModel = InAnimDataModel;
}

float UAnimSequenceBase::GetPlayLength() const
{
    if (AnimDataModel)
        return AnimDataModel->PlayLength;

    return 0.0f;
}

