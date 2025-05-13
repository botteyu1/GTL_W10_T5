#include "AnimDataModel.h"

UAnimDataModel::UAnimDataModel()
{
}

const TArray<FBoneAnimationTrack>& UAnimDataModel::GetBoneAnimationTracks() const
{
    return BoneAnimationTracks;
}

bool FAnimNotifyEvent::operator==(const FAnimNotifyEvent& Other) const
{
    return NotifyName == Other.NotifyName &&
        TriggerTime == Other.TriggerTime &&
        Duration == Other.Duration;
}
