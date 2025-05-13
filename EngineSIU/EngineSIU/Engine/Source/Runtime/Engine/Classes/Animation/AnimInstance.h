#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class USkeletalMeshComponent;
class UAnimationAsset;

struct FAnimationPlaybackContext
{
    UAnimationAsset* AnimationAsset = nullptr;
    float PreviousTime = 0.f;
    float PlaybackTime = 0.f;
    bool bIsLooping = false;
    float PlayRate = 1.f;
    float StartPosition = 0.f;
    float AnimationLength = 0.f;

    FAnimationPlaybackContext(UAnimationAsset* InAnimAsset, bool IsLoop = false, float InPlayRate = 1.f, float InStartPosition = 0.f);
};

// 애니메이션 상태를 나타내는 enum
enum class EAnimState
{
    Idle,
    Walk,
    Run,
    Fly
};

// Delegate 타입 선언 (이전 상태, 현재 상태 전달)
#include "Delegates/Delegate.h"
DECLARE_DELEGATE_TwoParams(FOnAnimStateChanged, EAnimState /*Previous*/, EAnimState /*Current*/);

// UAnimInstance: 추상 베이스 클래스
class UAnimInstance : public UObject
{
    DECLARE_CLASS(UAnimInstance, UObject);
public:
    UAnimInstance();
    virtual ~UAnimInstance();

    // 애니메이션 업데이트 함수
    void TriggerAnimNotifies(float DeltaTime);
    virtual void NativeUpdateAnimation(float DeltaTime);

    virtual void AddAnimationPlaybackContext(UAnimationAsset* InAnimAsset = nullptr, bool IsLoop = false, float InPlayRate = 1.f, float InStartPosition = 0.f);

    FAnimationPlaybackContext* GetAnimationPlaybackContext(UAnimationAsset* InAnimAsset);

    void Initialize(USkeletalMeshComponent* MeshComponent);
    USkeletalMeshComponent* GetSkeletalMeshComponent() const { return SkeletalMeshComponent; }
    void SetSkeletalMeshComponent(USkeletalMeshComponent* InSkeletalMeshComponent) { SkeletalMeshComponent = InSkeletalMeshComponent; }

    void ClearAnimationPlaybackContexts();


protected:
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
    TArray<std::shared_ptr<FAnimationPlaybackContext>> AnimationPlaybackContexts;
};
