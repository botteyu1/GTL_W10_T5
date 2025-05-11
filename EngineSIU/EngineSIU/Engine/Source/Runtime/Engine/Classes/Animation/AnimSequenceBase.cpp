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

FFrameRate UAnimSequenceBase::GetFrameRate() const
{
    {
        if (AnimDataModel)
            return AnimDataModel->FrameRate;

        return FFrameRate();
    }
}

const TArray<FBoneAnimationTrack>* UAnimSequenceBase::GetBoneAnimationTracks() const
{
    if (AnimDataModel)
    {
        const TArray<FBoneAnimationTrack>& tracksRef = AnimDataModel->GetBoneAnimationTracks();
        return &tracksRef; // 참조로부터 주소(&)를 가져와 포인터로 반환
    }
    return nullptr; 
}

void UAnimSequenceBase::AddAnimNotifyEvent(FAnimNotifyEvent Notify)
{
    Notifies.Add(Notify);
    SortNotifyEvents();
}

void UAnimSequenceBase::RemoveAnimNotifyEvent(FAnimNotifyEvent Notify)
{
    Notifies.Remove(Notify);
}

void UAnimSequenceBase::ClearAnimNotifyEvents()
{
    Notifies.Empty();
}

void UAnimSequenceBase::SortNotifyEvents()
{
    Notifies.Sort([](const FAnimNotifyEvent& A, const FAnimNotifyEvent& B)
        {
            return A.TriggerTime < B.TriggerTime;
        });
}

