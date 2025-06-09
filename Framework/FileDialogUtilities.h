#pragma once
#include "Game/Parameters/ParameterBuffers.h"
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>


class FileDialogUtilities
{
public:

    // テクスチャをロードして取得する
    static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetLoadTexture(ID3D11Device1* device);
	
    // JSONファイルを開いてロード
    template<typename T>
    static std::optional<T> OpenJsonFile()
    {
        char fileName[MAX_PATH] = "";

        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.hwndOwner = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (!GetOpenFileNameA(&ofn) || fileName[0] == '\0') {
            return std::nullopt;
        }

        std::ifstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "[FileDialogUtilities] Error: Failed to open file: " << fileName << std::endl;
            return std::nullopt;
        }

        try {
            nlohmann::json j;
            file >> j;

            return j.get<T>();
        }
        catch (const std::exception& e) {
            std::cerr << "[FileDialogUtilities] JSON parse error: " << e.what() << std::endl;
        }

        return std::nullopt;
    }
    // Jsonファイルをセーブする
    template<typename T>
    static std::string SaveJsonFile(const T& data)
    {
        char filename[MAX_PATH] = "params.json";

        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (!GetSaveFileNameA(&ofn) || filename[0] == '\0') {
            // キャンセルや選択失敗
            return "";
        }

        try {
            nlohmann::json j = data;
           
            std::ofstream out(filename);
            if (!out.is_open()) {
                std::cerr << "[SaveJsonFile] Failed to open file for writing: " << filename << std::endl;
                return "";
            }

            out << j.dump(4); // 整形して書き込む
            return filename;
        }
        catch (const std::exception& e) {
            std::cerr << "[SaveJsonFile] Exception: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "[SaveJsonFile] Unknown error occurred during JSON save." << std::endl;
        }

        return "";
    }


    // string から wstring への変換
	static std::wstring ConvertToWString(const std::string& str) {
		return std::wstring(str.begin(), str.end());
	};
};