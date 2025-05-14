#pragma once
#include "AnimInstance.h"
#include "Container/Map.h"

class UAnimSequence;

class UFSMAnimInstance : public UAnimInstance
{
    DECLARE_CLASS(UFSMAnimInstance, UAnimInstance)
public:
    UFSMAnimInstance();
    virtual ~UFSMAnimInstance() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;
    
    void AddAnimState(const FString& StateName, const FString& AnimName);
    void RemoveAnimState(const FString& StateName);
    void ChangeAnimState(const FString& CurrStateName, float BlendTime, bool bIsLooping);

    void SetPlaying(bool bInPlaying);
    bool IsPlaying() const { return bIsPlaying; }

protected:
    TMap<FName, UAnimSequence*> AnimStateMap; // 애니메이션 상태와 애니메이션 시퀀스 매핑

    UAnimSequence* CurrentAnimSequence = nullptr; // 현재 애니메이션 시퀀스

    bool bIsBlending = false;
    UAnimSequence* TargetAnimSequence = nullptr; // 목표 애니메이션 시퀀스
    float BlendDuration = 0.0f;// 블렌드 지속 시간
    float ElapsedBlendTime = 0.0f; // 경과된 블렌드 시간
    float CurrentBlendAlpha = 0.0f; // 현재 블렌드 알파 값

    // 현재 블렌드된 포즈에서 시작하기 위한 스냅샷
    bool bIsBlendingFromSnapshot = false;
    TArray<FTransform> SourceSnapshotPose;
    UAnimSequence* BlendSourceAnimForSnapshot = nullptr; // 블렌드 소스 애니메이션
    UAnimSequence* BlendTargetAnimForSnapshot = nullptr; // 블렌드 타겟 애니메이션
    float AlphaForSnapshot = 0.0f; // 스냅샷 블렌드 알파

    bool bIsPlaying = false; // 전체적인 재생 여부

private:
    void CaptureCurrentPoseAsSnapshot();
    void ApplyPoseToComponent(const TArray<FTransform>& LocalSpacePose);
};
