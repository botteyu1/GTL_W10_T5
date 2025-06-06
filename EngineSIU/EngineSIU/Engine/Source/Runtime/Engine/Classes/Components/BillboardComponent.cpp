#include "BillboardComponent.h"
#include <DirectXMath.h>
#include "Define.h"
#include "World/World.h"
#include "Actors/Player.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/MathUtility.h"
#include "UnrealEd/EditorViewportClient.h"
#include "EngineLoop.h"

UBillboardComponent::UBillboardComponent()
{
    SetType(StaticClass()->GetName());
    SetTexture(L"Assets/Editor/Icon/S_Actor.PNG");
}

UBillboardComponent::~UBillboardComponent()
{
}

UObject* UBillboardComponent::Duplicate(UObject* InOuter)
{
    // GPU 버퍼는 공유하지 않고, 상태 값만 복사하여 새로 초기화하도록 함
    UBillboardComponent* NewComponent = Cast<UBillboardComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->finalIndexU = finalIndexU;
        NewComponent->finalIndexV = finalIndexV;
        NewComponent->Texture = FEngineLoop::ResourceManager.GetTexture(TexturePath.ToWideString());
        NewComponent->TexturePath = TexturePath;
        NewComponent->UUIDParent = UUIDParent;
        NewComponent->bIsEditorBillboard = bIsEditorBillboard;
    }
    return NewComponent;
}

void UBillboardComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("FinalIndexU"), FString::Printf(TEXT("%f"), finalIndexU));
    OutProperties.Add(TEXT("FinalIndexV"), FString::Printf(TEXT("%f"), finalIndexV));
    OutProperties.Add(TEXT("BufferKey"), TexturePath);
}

void UBillboardComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("FinalIndexU"));
    if (TempStr)
    {
        finalIndexU = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("FinalIndexV"));
    if (TempStr)
    {
        finalIndexV = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("BufferKey"));
    if (TempStr)
    {
        TexturePath = *TempStr;
        Texture = FEngineLoop::ResourceManager.GetTexture(TempStr->ToWideString());
    }
}

void UBillboardComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void UBillboardComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int UBillboardComponent::CheckRayIntersection(const FVector& InRayOrigin, const FVector& InRayDirection, float& OutHitDistance) const
{
    TArray<FVector> Vertices =
    {
        FVector(-1.0f,  1.0f, 0.0f),
        FVector(1.0f,  1.0f, 0.0f),
        FVector(1.0f, -1.0f, 0.0f),
        FVector(-1.0f, -1.0f, 0.0f),
    };

    return CheckPickingOnNDC(Vertices, OutHitDistance) ? 1 : 0;
}

void UBillboardComponent::SetTexture(const FWString& InFilePath)
{
    Texture = FEngineLoop::ResourceManager.GetTexture(InFilePath);
    TexturePath = FString(InFilePath.c_str());
    //std::string str(_fileName.begin(), _fileName.end());
}

void UBillboardComponent::SetUUIDParent(USceneComponent* InUUIDParent)
{
    UUIDParent = InUUIDParent;
}

FMatrix UBillboardComponent::CreateBillboardMatrix() const
{
    // 카메라 뷰 행렬을 가져와서 위치 정보를 제거한 후 전치하여 LookAt 행렬 생성
    FMatrix CameraView = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    CameraView.M[0][3] = CameraView.M[1][3] = CameraView.M[2][3] = 0.0f;
    CameraView.M[3][0] = CameraView.M[3][1] = CameraView.M[3][2] = 0.0f;
    CameraView.M[3][3] = 1.0f;
    CameraView.M[0][2] = -CameraView.M[0][2];
    CameraView.M[1][2] = -CameraView.M[1][2];
    CameraView.M[2][2] = -CameraView.M[2][2];
    FMatrix LookAtCamera = FMatrix::Transpose(CameraView);

    FVector WorldLocation = GetComponentLocation();
    if (UUIDParent)
    {
        WorldLocation = UUIDParent->GetComponentLocation() + RelativeLocation;
    }
    
    FVector WorldScale = RelativeScale3D;
    FMatrix S = FMatrix::CreateScaleMatrix(WorldScale.X, WorldScale.Y, WorldScale.Z);
    FMatrix T = FMatrix::CreateTranslationMatrix(WorldLocation);
    
    // 최종 빌보드 행렬 = Scale * Rotation(LookAt) * Translation
    return S * LookAtCamera * T;
}


bool UBillboardComponent::CheckPickingOnNDC(const TArray<FVector>& quadVertices, float& hitDistance) const
{
    // TODO: 이 로직으로는 멀티 뷰포트에서 빌보드 피킹 안됨.
    
    // 마우스 위치를 클라이언트 좌표로 가져온 후 NDC 좌표로 변환
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(GEngineLoop.AppWnd, &mousePos);

    D3D11_VIEWPORT viewport;
    UINT numViewports = 1;
    FEngineLoop::GraphicDevice.DeviceContext->RSGetViewports(&numViewports, &viewport);

    // NDC 좌표 계산: X, Y는 [-1,1] 범위로 매핑
    float ndcX = (2.0f * mousePos.x / viewport.Width) - 1.0f;
    float ndcY = -((2.0f * mousePos.y / viewport.Height) - 1.0f);

    // MVP 행렬 계산
    std::shared_ptr<FEditorViewportClient> ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    FMatrix M = CreateBillboardMatrix();
    FMatrix V = ActiveViewport->GetViewMatrix();
    FMatrix P = ActiveViewport->GetProjectionMatrix();
    FMatrix MVP = M * V * P;

    // quadVertices를 MVP로 변환하여 NDC 공간에서의 최소/최대값 구하기
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    float avgZ = 0.0f;

    for (const FVector& v : quadVertices)
    {
        FVector4 clipPos = FMatrix::TransformVector(FVector4(v, 1.0f), MVP);
        if (clipPos.W != 0.0f)
        {
            clipPos = clipPos / clipPos.W;
        }
        minX = FMath::Min(minX, clipPos.X);
        maxX = FMath::Max(maxX, clipPos.X);
        minY = FMath::Min(minY, clipPos.Y);
        maxY = FMath::Max(maxY, clipPos.Y);
        avgZ += clipPos.Z;
    }

    avgZ /= quadVertices.Num();

    // 마우스 NDC 좌표가 quad의 NDC 경계 사각형 내에 있는지 검사
    if (ndcX >= minX && ndcX <= maxX && ndcY >= minY && ndcY <= maxY)
    {
        const FVector WorldLocation = GetComponentLocation();
        const FVector CameraLocation = ActiveViewport->GetCameraLocation();
        hitDistance = (WorldLocation - CameraLocation).Length();
        return true;
    }
    return false;
}
