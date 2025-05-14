
#include "SkeletalMeshComponent.h"

#include "ReferenceSkeleton.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimDataModel.h"
#include "UObject/ObjectFactory.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendAnimInstance.h"
#include "Animation/FSMAnimInstance.h"

USkeletalMeshComponent::USkeletalMeshComponent()
{
    AnimInstance = FObjectFactory::ConstructObject<UAnimSingleNodeInstance>(this);
    AnimInstance->Initialize(this);
}

USkeletalMeshComponent::~USkeletalMeshComponent()
{
}

UObject* USkeletalMeshComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->AnimInstance = static_cast<UAnimInstance*>(AnimInstance->Duplicate(InOuter));
    NewComponent->AnimInstance->Initialize(NewComponent);

    NewComponent->BoneTransforms = BoneTransforms;
    NewComponent->BoneBindPoseTransforms = BoneBindPoseTransforms;
    NewComponent->SkeletalMeshAsset = SkeletalMeshAsset;

    return NewComponent;
}

void USkeletalMeshComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void USkeletalMeshComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}

void USkeletalMeshComponent::TickComponent(float DeltaTime)
{
    USkinnedMeshComponent::TickComponent(DeltaTime);

    if(AnimInstance)
        AnimInstance->NativeUpdateAnimation(DeltaTime);
}

void USkeletalMeshComponent::SetSkeletalMeshAsset(USkeletalMesh* InSkeletalMeshAsset)
{
    SkeletalMeshAsset = InSkeletalMeshAsset;

    BoneTransforms.Empty();
    BoneBindPoseTransforms.Empty();
    
    const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();
    for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
    {
        BoneTransforms.Add(RefSkeleton.RawRefBonePose[i]);
        BoneBindPoseTransforms.Add(RefSkeleton.RawRefBonePose[i]);
    }
}

bool USkeletalMeshComponent::GetCurrentGlobalBoneMatrices(TArray<FMatrix>& OutBoneMatrices) const
{
    if (!SkeletalMeshAsset || !SkeletalMeshAsset->GetSkeleton())
    {
        return false;
    }
    const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();
    const TArray<FTransform>& BindPose = RefSkeleton.RawRefBonePose; // 로컬
    const int32 BoneNum = RefSkeleton.RawRefBoneInfo.Num();

    // 1. 현재 애니메이션 본 행렬 계산 (계층 구조 적용)
    OutBoneMatrices.Empty();
    OutBoneMatrices.SetNum(BoneNum);

    for (int32 BoneIndex = 0; BoneIndex < BoneNum; ++BoneIndex)
    {
        // 현재 본의 로컬 변환
        FTransform CurrentLocalTransform = BoneTransforms[BoneIndex];
        FMatrix LocalMatrix = CurrentLocalTransform.ToMatrixWithScale(); // FTransform -> FMatrix
        
        // 부모 본의 영향을 적용하여 월드 변환 구성
        int32 ParentIndex = RefSkeleton.RawRefBoneInfo[BoneIndex].ParentIndex;
        if (ParentIndex != INDEX_NONE)
        {
            // 로컬 변환에 부모 월드 변환 적용
            LocalMatrix = LocalMatrix * OutBoneMatrices[ParentIndex];
        }
        
        // 결과 행렬 저장
        OutBoneMatrices[BoneIndex] = LocalMatrix;
    }
    return true;
}

 void USkeletalMeshComponent::BeginPlay()
{
    USkinnedMeshComponent::BeginPlay();
    bPlayAnimation = true;
}

void USkeletalMeshComponent::SetAnimationEnabled(bool bEnable)
{
    bPlayAnimation = bEnable;

    if (UAnimSingleNodeInstance* SingleNodeInstance = Cast<UAnimSingleNodeInstance>(AnimInstance))
    {
        SingleNodeInstance->SetPlaying(bEnable);
    }

    if (UBlendAnimInstance* BlendAnimInstance = Cast<UBlendAnimInstance>(AnimInstance))
    {
        BlendAnimInstance->SetAnimationEnabled(bEnable);
    }

    if (!bPlayAnimation)
    {
        if (SkeletalMeshAsset && SkeletalMeshAsset->GetSkeleton())
        {
            const FReferenceSkeleton& RefSkeleton = SkeletalMeshAsset->GetSkeleton()->GetReferenceSkeleton();
            BoneTransforms = RefSkeleton.RawRefBonePose;
        }
    }
}

void USkeletalMeshComponent::SetAnimSequence(UAnimSequence* InAnimSequence)
{
    //AnimSequence = InAnimSequence;
    if (UAnimSingleNodeInstance* SingleNodeInstance = Cast<UAnimSingleNodeInstance>(AnimInstance))
    {
        SingleNodeInstance->SetAnimationAsset(InAnimSequence);
    }
}

void USkeletalMeshComponent::HandleAnimNotify(const FAnimNotifyEvent& Notify, ENotifyState NotifyState, float DeltaTime)
{
    GetOwner()->HandleAnimNotify(Notify, NotifyState, DeltaTime);
}

void USkeletalMeshComponent::SetAnimationTime(float InTime)
{
    AnimInstance->PauseAnimations();
    if (UAnimSingleNodeInstance* SingleNodeInstance = Cast<UAnimSingleNodeInstance>(AnimInstance))
    {
        SingleNodeInstance->SetAnimationTime(InTime);
    }
    ElapsedTime = InTime; 
}

void USkeletalMeshComponent::SetAnimInstanceClass(UClass* InstanceClass)
{
    if (Cast<UAnimSingleNodeInstance>(AnimInstance))
    {
        CachedAnimSingleNodeInstance = AnimInstance;
    }
    else if (Cast<UBlendAnimInstance>(AnimInstance))
    {
        CachedBlendAnimInstance = AnimInstance;
    }
    else
    {
        GUObjectArray.MarkRemoveObject(AnimInstance);
        AnimInstance = nullptr;
    }

    if (InstanceClass && InstanceClass == UAnimSingleNodeInstance::StaticClass())
    {
        if(CachedAnimSingleNodeInstance)
        {
            AnimInstance = CachedAnimSingleNodeInstance;
            CachedAnimSingleNodeInstance = nullptr;
        }
        else
        {
            AnimInstance = FObjectFactory::ConstructObject<UAnimSingleNodeInstance>(this);
            AnimInstance->Initialize(this);
        }
    }

    if (InstanceClass && InstanceClass == UBlendAnimInstance::StaticClass())
    {
        if (CachedBlendAnimInstance)
        {
            AnimInstance = CachedBlendAnimInstance;
            CachedBlendAnimInstance = nullptr;
        }
        else
        {
            AnimInstance = FObjectFactory::ConstructObject<UBlendAnimInstance>(this);
            AnimInstance->Initialize(this);
        }
    }

    if (InstanceClass && InstanceClass == UFSMAnimInstance::StaticClass())
    {
        AnimInstance = FObjectFactory::ConstructObject<UFSMAnimInstance>(this);
        AnimInstance->Initialize(this);
    }
}
