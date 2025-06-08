#include "pch.h"
#include "Framework/FileDialogUtilities.h"
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>


// JSONファイルを開くダイアログ（1つ選択）
std::string FileDialogUtilities::OpenJsonFile()
{
    // ファイル名バッファ
    char fileName[MAX_PATH] = "";

    // ファイルダイアログ構造体を初期化
    OPENFILENAMEA ofn = {};
    // 構造体サイズ
    ofn.lStructSize = sizeof(ofn);
    // 結果として選択されたファイルパスが入るバッファ
    ofn.lpstrFile = fileName;
    // バッファのサイズ
    ofn.nMaxFile = MAX_PATH;

    // ファイル選択時のフィルター
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    // 最初のフィルターをデフォルト
    ofn.nFilterIndex = 1;
    // 親ウィンドウ
    ofn.hwndOwner = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST |  // パスが存在する必要あり
        OFN_FILEMUSTEXIST;          // ファイルが存在する必要あり

    // ファイルを開くダイアログを表示し、選択された場合のみファイルパスを返す
    if (GetOpenFileNameA(&ofn)) {
        return fileName;
    }

    // キャンセルやエラー時は空文字列を返す
    return "";
}

// JSONファイルを保存するダイアログ（上書き確認あり）
std::string FileDialogUtilities::SaveJsonFile()
{
    char filename[MAX_PATH] = "params.json";

    OPENFILENAMEA ofn = {};

    ofn.lStructSize = sizeof(ofn);

    ofn.lpstrFile = filename;

    ofn.nMaxFile = MAX_PATH;

    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";

    ofn.nFilterIndex = 1;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) 
    {
        return filename;
    }

    // キャンセルやエラー時は空文字列を返す
    return "";
}