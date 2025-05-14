#pragma once
#include <Windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <Shellapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Shell32.lib")


namespace LuaScriptFileUtils
{
    // template.lua → Scene_Actor.lua 로 복제
    inline bool MakeScriptPathAndDisplayName(
        const std::wstring& templateName,
        const std::wstring& sceneName,
        const std::wstring& actorName,
        FString& outScriptPath,
        FString& outScriptName
    )
    {
        LPCWSTR luaDir = L"LuaScripts";

        wchar_t src[MAX_PATH] = { 0 };
        PathCombineW(src, luaDir, templateName.c_str());
        if (!PathFileExistsW(src))
            return false;

        // 대상 파일명: Scene_Actor.lua
        std::wstring destName = sceneName + L"_" + actorName + L".lua";
        outScriptName = FString(destName.c_str());

        wchar_t dst[MAX_PATH] = { 0 };
        PathCombineW(dst, luaDir, destName.c_str());

        // 복제 (덮어쓰기 허용)
        /*if (!CopyFileW(src, dst, FALSE))
            return false;*/

        outScriptPath = FString(dst);
        return true;
    }



    inline bool FileExistsSimple(const LPCTSTR filePath) {
        DWORD fileAttributes = GetFileAttributes(filePath);
        // 파일 속성을 가져올 수 있고, 디렉터리가 아니어야 파일로 간주
        return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
            !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
    }

    // 지정된 경로에 Lua 파일이 없으면 템플릿으로 생성
// 반환값: 성공 시 true, 실패 시 false
    inline bool CreateLuaScriptFromTemplateIfNeeded(const LPCTSTR InLuaFilePath)
    {
        const LPCTSTR templateFilePath = _T("LuaScripts/template.lua");

        // 1. 대상 Lua 파일이 존재하는지 확인
        if (FileExistsSimple(InLuaFilePath))
        {
            // 이미 파일이 존재하면 아무것도 하지 않고 성공으로 간주 (또는 다른 동작 정의 가능)
            return true;
        }

        // 2. 대상 파일이 존재하지 않으면 템플릿으로부터 복사하여 생성 시도
        // 2a. 템플릿 파일이 존재하는지 먼저 확인
        if (!FileExistsSimple(templateFilePath))
        {
            // 템플릿 파일이 없으면 생성 불가
            // 간단한 로그 또는 오류 처리 (여기서는 반환값으로 실패 알림)
            // OutputDebugString(_T("Error: Template file (LuaScripts/template.lua) not found.\n"));
            return false;
        }

        // 2b. 템플릿 파일을 목표 경로로 복사
        // CopyFile의 세 번째 인자 bFailIfExists는 TRUE로 설정하여,
        // 만약 대상 파일이 (매우 짧은 시간 사이에 다른 프로세스에 의해) 이미 생성되었다면 실패하도록 함.
        // 여기서는 FileExistsSimple로 이미 확인했으므로 FALSE로 해도 무방하나, 안전을 위해 TRUE 사용 가능.
        // 여기서는 이미 FileExistsSimple로 확인했으므로 FALSE 사용 (덮어쓰지 않음).
        if (!CopyFile(templateFilePath, InLuaFilePath, FALSE))
        {
            // 파일 복사 실패
            // 간단한 로그 또는 오류 처리
            // TCHAR errMsg[256];
            // wsprintf(errMsg, _T("Error: Failed to copy template file to %s. Error Code: %lu\n"), InLuaFilePath, GetLastError());
            // OutputDebugString(errMsg);
            return false;
        }

        // 파일 생성 성공
        return true;
    }

    // 기본 에디터(ShellExecute)로 Lua 파일 열기
    inline void OpenLuaScriptFile(const LPCTSTR InLuaFilePath)
    {
        HINSTANCE hInst = ShellExecute(
            NULL,
            _T("open"),
            InLuaFilePath,
            NULL,
            NULL,
            SW_SHOWNORMAL
        );

        if ((INT_PTR)hInst <= 32) {
            //MessageBox(NULL, _T("파일 열기에 실패했습니다."), _T("Error"), MB_OK | MB_ICONERROR);
            if (CreateLuaScriptFromTemplateIfNeeded(InLuaFilePath))
            {
                // 파일이 없으면 템플릿으로 생성
                hInst = ShellExecute(
                    NULL,
                    _T("open"),
                    InLuaFilePath,
                    NULL,
                    NULL,
                    SW_SHOWNORMAL
                );
            }
            else
            {
                // 템플릿 생성 실패
                //MessageBox(NULL, _T("템플릿 파일 생성에 실패했습니다."), _T("Error"), MB_OK | MB_ICONERROR);
            }
        }
    }
}
