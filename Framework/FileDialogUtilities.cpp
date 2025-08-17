#include "pch.h"
#include "Framework/FileDialogUtilities.h"
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <WICTextureLoader.h>

std::string FileDialogUtilities::s_filePath = "";

/// <summary>
/// テクスチャをロード取得する
/// </summary>
/// <param name="device">デバイス</param>
/// <returns>テクスチャ</returns>
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> FileDialogUtilities::GetLoadTexture(ID3D11Device1* device)
{
    // ファイル名バッファ
    char fileName[MAX_PATH] = "";

    // ファイルダイアログ構造体を初期化
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Image Files (*.png; *.jpg)\0*.png;*.jpg\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.hwndOwner = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // ユーザーがキャンセルした、または選択失敗
    if (!GetOpenFileNameA(&ofn) || fileName[0] == '\0') 
    {
        return nullptr;
    }

    // パスをwstringに変換
    std::wstring wPath = FileDialogUtilities::ConvertToWString(fileName);

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;

    HRESULT hr = DirectX::CreateWICTextureFromFile(
        device,
        wPath.c_str(),
        nullptr,
        texture.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        // ロードに失敗した場合メッセージを表示
        MessageBoxA(NULL, ("Failed to load texture:\n" + std::string(fileName)).c_str(),
            "Texture Load Error", MB_ICONERROR);

        // 失敗の場合nullを返す
        return nullptr;
    }

    return texture;
}
