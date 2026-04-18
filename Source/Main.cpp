#include "Pch.hpp"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved)
{
    switch (ulReasonForCall)
    {
        case DLL_PROCESS_ATTACH:
            // DLLがプロセスにロードされたときの初期化処理
            break;

        case DLL_THREAD_ATTACH:
            // スレッドが作成されたときの処理（必要に応じて）
            break;

        case DLL_THREAD_DETACH:
            // スレッドが終了したときの処理（必要に応じて）
            break;

        case DLL_PROCESS_DETACH:
            // DLLがプロセスからアンロードされるときのクリーンアップ処理
            break;
    }

    return TRUE;
}
