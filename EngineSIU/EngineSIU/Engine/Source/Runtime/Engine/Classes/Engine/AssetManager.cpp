#include "AssetManager.h"
#include "Engine.h"

#include <filesystem>

#include "FbxLoader.h"
#include "Animation/Skeleton.h"
#include "SkeletalMesh.h"
#include "Components/Material/Material.h"
#include "Engine/FObjLoader.h"
#include "UObject/Casts.h"
#include "Asset/SkeletalMeshAsset.h"

UAssetManager::~UAssetManager()
{
    for (auto& [Name, Object] : SkeletonMap)
    {
        if (Object)
        {
            delete Object;
            Object = nullptr;
        }
    }
    SkeletonMap.Empty();
    
    for (auto& [Name, Object] : SkeletalMeshMap)
    {
        if (Object)
        {
            delete Object;
            Object = nullptr;
        }
    }
    SkeletalMeshMap.Empty();

    for (auto& [Name, Object] : StaticMeshMap)
    {
        if (Object)
        {
            delete Object;
            Object = nullptr;
        }
    }
    StaticMeshMap.Empty();

    for (auto& [Name, Object] : MaterialMap)
    {
        if (Object)
        {
            delete Object;
            Object = nullptr;
        }
    }
    MaterialMap.Empty();
}

bool UAssetManager::IsInitialized()
{
    return GEngine && GEngine->AssetManager;
}

UAssetManager& UAssetManager::Get()
{
    if (UAssetManager* Singleton = GEngine->AssetManager)
    {
        return *Singleton;
    }
    else
    {
        UE_LOG(ELogLevel::Error, "Cannot use AssetManager if no AssetManagerClassName is defined!");
        assert(0);
        return *new UAssetManager; // never calls this
    }
}

UAssetManager* UAssetManager::GetIfInitialized()
{
    return GEngine ? GEngine->AssetManager : nullptr;
}

void UAssetManager::InitAssetManager()
{
    AssetRegistry = std::make_unique<FAssetRegistry>();

    LoadContentFiles();
}

const TMap<FName, FAssetInfo>& UAssetManager::GetAssetRegistry()
{
    return AssetRegistry->PathNameToAssetInfo;
}

TMap<FName, FAssetInfo>& UAssetManager::GetAssetRegistryRef()
{
    return AssetRegistry->PathNameToAssetInfo;
}

USkeletalMesh* UAssetManager::GetSkeletalMesh(const FName& Name)
{
    if (SkeletalMeshMap.Contains(Name))
    {
        return SkeletalMeshMap[Name];
    }
    return nullptr;
}

UStaticMesh* UAssetManager::GetStaticMesh(const FName& Name)
{
    if (StaticMeshMap.Contains(Name))
    {
        return StaticMeshMap[Name];
    }
    return nullptr;
}

USkeleton* UAssetManager::GetSkeleton(const FName& Name)
{
    if (SkeletonMap.Contains(Name))
    {
        return SkeletonMap[Name];
    }
    return nullptr;
}

UMaterial* UAssetManager::GetMaterial(const FName& Name)
{
    if (MaterialMap.Contains(Name))
    {
        return MaterialMap[Name];
    }
    return nullptr;
}

void UAssetManager::AddAssetInfo(const FAssetInfo& Info)
{
    AssetRegistry->PathNameToAssetInfo.Add(Info.AssetName, Info);
}

void UAssetManager::AddSkeleton(const FName& Key, USkeleton* Skeleton)
{
    SkeletonMap.Add(Key, Skeleton);
}

void UAssetManager::AddSkeletalMesh(const FName& Key, USkeletalMesh* Mesh)
{
    SkeletalMeshMap.Add(Key, Mesh);
}

void UAssetManager::AddMaterial(const FName& Key, UMaterial* Material)
{
    MaterialMap.Add(Key, Material);
}

void UAssetManager::LoadContentFiles()
{
    const std::string BasePathName = "Contents/";

    // Obj 파일 로드
    
    for (const auto& Entry : std::filesystem::recursive_directory_iterator(BasePathName))
    {
        if (Entry.is_regular_file() && Entry.path().extension() == ".obj")
        {
            FAssetInfo NewAssetInfo;
            NewAssetInfo.AssetName = FName(Entry.path().filename().string());
            NewAssetInfo.PackagePath = FName(Entry.path().parent_path().string());
            NewAssetInfo.AssetType = EAssetType::StaticMesh; // obj 파일은 무조건 StaticMesh
            NewAssetInfo.Size = static_cast<uint32>(std::filesystem::file_size(Entry.path()));
            
            AssetRegistry->PathNameToAssetInfo.Add(NewAssetInfo.AssetName, NewAssetInfo);
            
            FString MeshName = NewAssetInfo.PackagePath.ToString() + "/" + NewAssetInfo.AssetName.ToString();
            FObjManager::CreateStaticMesh(MeshName);
            // ObjFileNames.push_back(UGTLStringLibrary::StringToWString(Entry.path().string()));
            // FObjManager::LoadObjStaticMeshAsset(UGTLStringLibrary::StringToWString(Entry.path().string()));
        }
        else if (Entry.is_regular_file() && (Entry.path().extension() == ".fbx" || Entry.path().extension() == ".FBX"))
        {
            // TODO : ControlEditorPanel Viwer Open과 코드 중복 다수
            // 경로, 이름 준비
            const FString FilePath = Entry.path().parent_path().string() + "/" + Entry.path().filename().string();
            const FString FileNameWithoutExt = Entry.path().stem().filename().string();

            // FBX 로더로 파일 읽기
            FFbxLoader Loader;
            FFbxLoadResult Result = Loader.LoadFBX(FilePath);

            // AssetInfo 기본 필드 세팅
            FAssetInfo AssetInfo = {};
            AssetInfo.PackagePath = FName(Entry.path().parent_path().wstring());
            AssetInfo.Size = static_cast<uint32>(std::filesystem::file_size(Entry.path()));

            // 로드된 skeleton 등록
            for (int32 i = 0; i < Result.Skeletons.Num(); ++i)
            {
                USkeleton* Skeleton = Result.Skeletons[i];
                FString BaseAssetName = FileNameWithoutExt + "_Skeleton";
                
                FAssetInfo Info = AssetInfo;
                Info.AssetName = i > 0 ? FName(BaseAssetName + FString::FromInt(i)) : FName(BaseAssetName);
                Info.AssetType = EAssetType::Skeleton;
                AssetRegistry->PathNameToAssetInfo.Add(Info.AssetName, Info);

                FString Key = Info.PackagePath.ToString() + "/" + Info.AssetName.ToString();
                SkeletonMap.Add(Key, Skeleton);
            }

            // 로드된 SkeletalMesh 등록
            for (int32 i = 0; i < Result.SkeletalMeshes.Num(); ++i)
            {
                USkeletalMesh* SkeletalMesh = Result.SkeletalMeshes[i];
                FString BaseAssetName = FileNameWithoutExt;
                
                FAssetInfo Info = AssetInfo;
                Info.AssetName = i > 0 ? FName(BaseAssetName + FString::FromInt(i)) : FName(BaseAssetName);
                Info.AssetType = EAssetType::SkeletalMesh;
                AssetRegistry->PathNameToAssetInfo.Add(Info.AssetName, Info);

                FString Key = Info.PackagePath.ToString() + "/" + Info.AssetName.ToString();
                SkeletalMeshMap.Add(Key, SkeletalMesh);
            }
            for (int32 i = 0; i < Result.StaticMeshes.Num(); ++i)
            {
                UStaticMesh* StaticMesh = Result.StaticMeshes[i];
                FString BaseAssetName = FileNameWithoutExt;
                
                FAssetInfo Info = AssetInfo;
                Info.AssetName = i > 0 ? FName(BaseAssetName + FString::FromInt(i)) : FName(BaseAssetName);
                Info.AssetType = EAssetType::StaticMesh;
                AssetRegistry->PathNameToAssetInfo.Add(Info.AssetName, Info);

                FString Key = Info.PackagePath.ToString() + "/" + Info.AssetName.ToString();
                StaticMeshMap.Add(Key, StaticMesh);
            }
            for (int32 i = 0; i < Result.Materials.Num(); ++i)
            {
                UMaterial* Material = Result.Materials[i];
                FString BaseAssetName = Material->GetName();
                
                FAssetInfo Info = AssetInfo;
                Info.AssetName = FName(BaseAssetName);
                Info.AssetType = EAssetType::Material;
                AssetRegistry->PathNameToAssetInfo.Add(Info.AssetName, Info);

                FString Key = Info.PackagePath.ToString() + "/" + Info.AssetName.ToString();
                MaterialMap.Add(Key, Material);
            }
        }
    }
}
