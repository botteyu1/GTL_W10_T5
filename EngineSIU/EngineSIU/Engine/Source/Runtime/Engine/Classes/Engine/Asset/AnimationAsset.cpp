#include "AnimationAsset.h"
#include "Classes/Animation/Skeleton.h"

void UAnimationAsset::SetSkeleton(USkeleton* Skeleton)
{
    this->Skeleton = Skeleton;
}
