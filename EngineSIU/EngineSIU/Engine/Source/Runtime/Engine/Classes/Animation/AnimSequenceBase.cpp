#include "AnimSequenceBase.h"

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
