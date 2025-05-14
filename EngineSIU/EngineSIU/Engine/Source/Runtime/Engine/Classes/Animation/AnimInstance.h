#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class USkeletalMeshComponent;
class UAnimationAsset;
struct FTransform;

struct FAnimationPlaybackContext
{
    UAnimationAsset* AnimationAsset = nullptr;
    float PreviousTime = 0.f;
    float PlaybackTime = 0.f;
    bool bIsLooping = false; 
    float PlayRate = 1.f;
    float StartPosition = 0.f;
    float AnimationLength = 0.f;
    bool bIsPlaying = false; // Pause 여부 처리. Stop시에는 그냥 배열에서 제거

    FAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop = false, float InPlayRate = 1.f, float InStartPosition = 0.f);
};

// UAnimInstance: 추상 베이스 클래스
class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject);
public:
    UAnimInstance();
    virtual ~UAnimInstance();

    UObject* Duplicate(UObject* InOuter) override;
    // 애니메이션 업데이트 함수
    virtual void TriggerAnimNotifies(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaTime);

    virtual void AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset = nullptr, bool IsLoop = false, float InPlayRate = 1.f, float InStartPosition = 0.f);

    FAnimationPlaybackContext* GetAnimationPlaybackContext(UAnimationAsset* InAnimAsset);

    virtual void Initialize(USkeletalMeshComponent* MeshComponent);
    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }
    void SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComponent) { SkeletalMeshComponent = InSkeletalMeshComponent; }

    void ClearAnimationPlaybackContexts();

    void PauseAnimations();

protected:
    FTransform GetCurrentAnimatedTransform(UAnimationAsset* AnimInstance, FName BoneName);

protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
    TArray<std::shared_ptr<FAnimationPlaybackContext>> AnimationPlaybackContexts;
};
