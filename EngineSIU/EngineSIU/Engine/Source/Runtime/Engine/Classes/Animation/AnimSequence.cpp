
#include "AnimSequence.h"

#include "Math/Transform.h"
#include "Animation/AnimDataModel.h"

UAnimSequence::UAnimSequence()
{
    
}

FName UAnimSequence::GetName() const
{
    return AnimDataModel ? AnimDataModel->Name : NAME_None;
}

