#include "pch.h"
#include "Framework/FileDialogUtilities.h"
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>


// JSON�t�@�C�����J���_�C�A���O�i1�I���j
std::string FileDialogUtilities::OpenJsonFile()
{
    // �t�@�C�����o�b�t�@
    char fileName[MAX_PATH] = "";

    // �t�@�C���_�C�A���O�\���̂�������
    OPENFILENAMEA ofn = {};
    // �\���̃T�C�Y
    ofn.lStructSize = sizeof(ofn);
    // ���ʂƂ��đI�����ꂽ�t�@�C���p�X������o�b�t�@
    ofn.lpstrFile = fileName;
    // �o�b�t�@�̃T�C�Y
    ofn.nMaxFile = MAX_PATH;

    // �t�@�C���I�����̃t�B���^�[
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    // �ŏ��̃t�B���^�[���f�t�H���g
    ofn.nFilterIndex = 1;
    // �e�E�B���h�E
    ofn.hwndOwner = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST |  // �p�X�����݂���K�v����
        OFN_FILEMUSTEXIST;          // �t�@�C�������݂���K�v����

    // �t�@�C�����J���_�C�A���O��\�����A�I�����ꂽ�ꍇ�̂݃t�@�C���p�X��Ԃ�
    if (GetOpenFileNameA(&ofn)) {
        return fileName;
    }

    // �L�����Z����G���[���͋󕶎����Ԃ�
    return "";
}

// JSON�t�@�C����ۑ�����_�C�A���O�i�㏑���m�F����j
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

    // �L�����Z����G���[���͋󕶎����Ԃ�
    return "";
}