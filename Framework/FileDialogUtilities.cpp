#include "pch.h"
#include "Framework/FileDialogUtilities.h"
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <WICTextureLoader.h>

std::string FileDialogUtilities::s_filePath = "";

/// <summary>
/// �e�N�X�`�������[�h�擾����
/// </summary>
/// <param name="device">�f�o�C�X</param>
/// <returns>�e�N�X�`��</returns>
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> FileDialogUtilities::GetLoadTexture(ID3D11Device1* device)
{
    // �t�@�C�����o�b�t�@
    char fileName[MAX_PATH] = "";

    // �t�@�C���_�C�A���O�\���̂�������
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Image Files (*.png; *.jpg)\0*.png;*.jpg\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.hwndOwner = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // ���[�U�[���L�����Z�������A�܂��͑I�����s
    if (!GetOpenFileNameA(&ofn) || fileName[0] == '\0') 
    {
        return nullptr;
    }

    // �p�X��wstring�ɕϊ�
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
        // ���[�h�Ɏ��s�����ꍇ���b�Z�[�W��\��
        MessageBoxA(NULL, ("Failed to load texture:\n" + std::string(fileName)).c_str(),
            "Texture Load Error", MB_ICONERROR);

        // ���s�̏ꍇnull��Ԃ�
        return nullptr;
    }

    return texture;
}
