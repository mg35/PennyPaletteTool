// PennyPaletteTool.cpp : Defines the entry point for the application.
//
#pragma comment( lib, "Msimg32.lib" )

#include "framework.h"
#include "PennyPaletteTool.h"
#include <string>
#include <vector>
#include <filesystem>
#include "ShObjIdl_core.h"
#include <WinUser.h>
#include "wingdi.h"
#include <fstream>
#include <iostream>
#include <Windows.h>

#define MAX_LOADSTRING 100

#define OPEN_ID 123
#define SAVE_ID 124
#define MESH_CATEG_COMBOBOX_ID 125
#define MESH_FILE_COMBOBOX_ID 126
#define PALETTE_1_COMBOBOX_ID 127
#define PALETTE_2_COMBOBOX_ID 128
#define INFO_TEXT_1_ID 129
#define INFO_TEXT_2_ID 130
#define INFO_TEXT_3_ID 131

#define MESH_CATEG_DROPDOWN_WIDTH 130
#define MESH_CATEG_DROPDOWN_TOP 75
#define MESH_FILE_DROPDOWN_WIDTH 200
#define MESH_FILE_DROPDOWN_TOP 115


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PENNYPALETTETOOL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PENNYPALETTETOOL));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PENNYPALETTETOOL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PENNYPALETTETOOL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

HWND infoText1;
HWND meshCategoryComboBox;
HWND meshFileComboBox;
HWND palette1ComboBox;
HWND palette2ComboBox;
HWND openFolderButton;
std::vector<std::wstring> meshCategories;
std::vector<std::wstring> meshFiles;
std::vector<HWND> textElementHandles;
std::vector<HWND> paletteNameComboBoxes;
std::vector<HWND> paletteFileComboBoxes;
std::wstring basePath = LR"(C:\Program Files (x86)\Steam\steamapps\common\PennysBigBreakaway\Data\Meshes)";
std::string basePalettePath = R"(C:\Program Files (x86)\Steam\steamapps\common\PennysBigBreakaway\Data\Palettes)";
std::wstring wBasePalettePath = LR"(C:\Program Files (x86)\Steam\steamapps\common\PennysBigBreakaway\Data\Palettes)";
char* meshStartBuf;
int meshStartBufLen;
char* meshEndBuf;
int meshEndBufLen;

void getMeshCategories() {
    meshCategories.clear();
    for (const auto& dirEntry : std::filesystem::directory_iterator(basePath)) {
        if (dirEntry.is_directory()) {
            meshCategories.push_back(dirEntry.path().filename().generic_wstring());
        }
    }
}

void getMeshFiles(std::wstring meshDir) {
    meshFiles.clear();
    for (const auto& dirEntry : std::filesystem::directory_iterator(basePath + L"\\" + meshDir)) {
        if (dirEntry.path().extension().compare(".bin") == 0)
        meshFiles.push_back(dirEntry.path().filename().generic_wstring());
    }
}

void getPaletteFiles(std::vector<std::wstring> *paletteFiles) {
    paletteFiles->clear();
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(basePalettePath)) {
        if (dirEntry.path().extension().compare(".bin") == 0)
            paletteFiles->push_back(dirEntry.path().generic_wstring().substr(basePalettePath.size() + 1));
    }
}

void getPalettes(std::vector<std::wstring> *palettes, std::wstring paletteFile) {
    palettes->clear();
    std::ifstream reader;
    char charBuf[8];
    std::wstring filePath = wBasePalettePath + L"\\" + paletteFile;
    reader.open(filePath.c_str(), std::ios::binary);
    reader.read(charBuf, 4);
    reader.read(charBuf, 1);
    int numChannels = charBuf[0];
    reader.read(charBuf, 1);
    int numPalettes = charBuf[0];
    int paletteCounter = 0;
    std::string curPaletteName;
    while (paletteCounter <= numPalettes) {
        std::getline(reader, curPaletteName, '\0');
        int count = MultiByteToWideChar(CP_ACP, 0, curPaletteName.c_str(), curPaletteName.length(), NULL, 0);
        std::wstring wPaletteName(count, 0);
        MultiByteToWideChar(CP_ACP, 0, curPaletteName.c_str(), curPaletteName.length(), &wPaletteName[0], count);
        palettes->push_back(wPaletteName);
        int channelCounter = 0;
        while (channelCounter < numChannels) {
            reader.read(charBuf, 2);
            short numStops = *(short*)charBuf;
            int stopCounter = 0;
            while (stopCounter < numStops) {
                reader.read(charBuf, 8);
                stopCounter += 1;
            }
            channelCounter += 1;
        }
        paletteCounter += 1;
    }
    reader.close();
}

PWSTR loadFolder() {
    IFileOpenDialog* pFolderOpen;
    PWSTR pszFilePath = NULL;

    // Create the FileOpenDialog object.
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFolderOpen));
    if (SUCCEEDED(hr))
    {
        hr = pFolderOpen->SetOptions(FOS_PICKFOLDERS);

        if (SUCCEEDED(hr))
        {
            hr = pFolderOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFolderOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    pItem->Release();
                }
            }
            pFolderOpen->Release();
        }
    }
    return pszFilePath;
}

void initInfoText1(HWND hWnd) {
    infoText1 = CreateWindow(WC_STATIC, L"If the dropdown is empty, then", WS_VISIBLE | WS_CHILD | SS_LEFT,
        0, 0, 130, 35, hWnd, (HMENU)INFO_TEXT_1_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void initInfoText2(HWND hWnd) {
    infoText1 = CreateWindow(WC_STATIC, L"Model Folder:", WS_VISIBLE | WS_CHILD | SS_LEFT,
        0, 55, 130, 20, hWnd, (HMENU)INFO_TEXT_2_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void initInfoText3(HWND hWnd) {
    infoText1 = CreateWindow(WC_STATIC, L"Model File:", WS_VISIBLE | WS_CHILD | SS_LEFT,
        0, 95, 130, 20, hWnd, (HMENU)INFO_TEXT_3_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void initOpenFolderButton(HWND hWnd) {
    openFolderButton = CreateWindow(
        L"BUTTON", L"Choose Base Path", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 35, 130, 20, hWnd, (HMENU)OPEN_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void initMeshCategoryDropdown(HWND hWnd) {
    meshCategoryComboBox = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        0, MESH_CATEG_DROPDOWN_TOP, MESH_CATEG_DROPDOWN_WIDTH, 26 + 16 * meshCategories.size(), hWnd, (HMENU)MESH_CATEG_COMBOBOX_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);
}

void initMeshFileDropdown(HWND hWnd) {
    meshFileComboBox = CreateWindow(WC_COMBOBOX, TEXT(""),
        CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        0, MESH_FILE_DROPDOWN_TOP, MESH_FILE_DROPDOWN_WIDTH, 26 + 16 * meshFiles.size(), hWnd, (HMENU)MESH_FILE_COMBOBOX_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL);
}

void initSaveMeshButton(HWND hWnd) {
    openFolderButton = CreateWindow(
        L"BUTTON", L"Save Palettes", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 150, 130, 20, hWnd, (HMENU)SAVE_ID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
}

void renderPalette(std::wstring wPaletteName, std::wstring wPaletteFile, int x, int y, HWND hWnd, HDC hdc) {
    std::ifstream reader;
    char charBuf[8];
    std::wstring filePath = wBasePalettePath + L"\\" + wPaletteFile;
    reader.open(filePath.c_str(), std::ios::binary);
    reader.read(charBuf, 4);
    reader.read(charBuf, 1);
    int numChannels = charBuf[0];
    reader.read(charBuf, 1);
    int numPalettes = charBuf[0];
    int paletteCounter = 0;
    std::string curPaletteName;
    while (paletteCounter <= numPalettes) {
        std::getline(reader, curPaletteName, '\0');

        int count = MultiByteToWideChar(CP_ACP, 0, curPaletteName.c_str(), curPaletteName.length(), NULL, 0);
        std::wstring wCurPaletteName(count, 0);
        MultiByteToWideChar(CP_ACP, 0, curPaletteName.c_str(), curPaletteName.length(), &wCurPaletteName[0], count);
        int channelCounter = 0;
        if (wCurPaletteName.compare(wPaletteName) == 0) {
            TRIVERTEX colors[32];
            LONG numColors = 0;
            GRADIENT_RECT rects[32];
            ULONG numRects = 0;
            while (channelCounter < numChannels) {
                reader.read(charBuf, 2);
                short numStops = *(short*)charBuf;
                int stopCounter = 0;
                while (stopCounter < numStops) {
                    float colorPos;
                    float alpha;
                    unsigned short rgba[4];
                    reader.read(charBuf, 4);
                    colorPos = *(float*)charBuf;
                    reader.read(charBuf, 4);
                    rgba[0] = charBuf[2] << 8;
                    rgba[1] = charBuf[1] << 8;
                    rgba[2] = charBuf[0] << 8;
                    rgba[3] = charBuf[3] << 8;
                    colors[numColors] = { (long)(colorPos * 150) + x, numColors % 2 * 25 + y, rgba[0], rgba[1], rgba[2], rgba[3] };
                    numColors += 1;
                    if (stopCounter < numStops - 1) {
                        rects[numRects] = { numRects, numRects + 1 };
                        numRects += 1;
                    }
                    stopCounter += 1;
                }
                channelCounter += 1;
                break;
            }
            GradientFill(hdc, colors, numColors, rects, numRects, GRADIENT_FILL_RECT_H);
            break;
        }
        else {
            while (channelCounter < numChannels) {
                reader.read(charBuf, 2);
                short numStops = *(short*)charBuf;
                int stopCounter = 0;
                while (stopCounter < numStops) {
                    reader.read(charBuf, 8);
                    stopCounter += 1;
                }
                channelCounter += 1;
            }
        }
        paletteCounter += 1;
    }
    reader.close();
}

void updatePaletteColorDisplay2(HWND hWnd, HDC hdc) {
    for (int i = 0; i < paletteFileComboBoxes.size(); i++) {

        int ItemIndex = SendMessage(paletteFileComboBoxes.at(i), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        WCHAR wPaletteFile[256];
        (WCHAR)SendMessage(paletteFileComboBoxes.at(i), (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)wPaletteFile);
        ItemIndex = SendMessage(paletteNameComboBoxes.at(i), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        WCHAR wPaletteName[256];
        (WCHAR)SendMessage(paletteNameComboBoxes.at(i), (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)wPaletteName);

        renderPalette(wPaletteName, wPaletteFile, 500, 50 + i * 80 + 50, hWnd, hdc);
    }
}

void updatePaletteColorDisplay(HWND hWnd, HDC hdc) {
    int ItemIndex = SendMessage(meshCategoryComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR categoryItem[256];
    (WCHAR)SendMessage(meshCategoryComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)categoryItem);
    ItemIndex = SendMessage(meshFileComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR fileItem[256];
    (WCHAR)SendMessage(meshFileComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)fileItem);
    std::ifstream reader;
    char charBuf[12];
    int numFaces = 0;
    int numVerts = 0;
    std::wstring filePath = basePath + L"\\" + categoryItem + L"\\" + fileItem;
    reader.open(filePath.c_str(), std::ios::binary);
    reader.read(charBuf, 12);
    reader.read(charBuf, 4);
    numVerts = *(int*)charBuf;
    reader.read(charBuf, 4);
    numFaces = (*(int*)charBuf) / 3;
    int paletteOffset = 0x1F + numFaces * 6 + numVerts * 12;
    meshStartBufLen = paletteOffset + 1;
    reader.seekg(0);
    if (meshStartBuf != NULL) {
        free(meshStartBuf);
    }
    meshStartBuf = (char*)malloc(meshStartBufLen);
    reader.read(meshStartBuf, meshStartBufLen);
    reader.seekg(paletteOffset);
    reader.read(charBuf, 1);
    int numPalettes = charBuf[0];
    int textY = 50;
    std::string paletteName;
    std::string paletteFile;

    for (int i = 0; i < numPalettes; i++) {
        std::getline(reader, paletteName, '\0');
        std::string newPaletteFile;
        std::getline(reader, newPaletteFile, '\0');
        if (!newPaletteFile.empty()) {
            paletteFile = newPaletteFile;
        }

        int count = MultiByteToWideChar(CP_ACP, 0, paletteName.c_str(), paletteName.length(), NULL, 0);
        std::wstring wPaletteName(count, 0);
        MultiByteToWideChar(CP_ACP, 0, paletteName.c_str(), paletteName.length(), &wPaletteName[0], count);

        count = MultiByteToWideChar(CP_ACP, 0, paletteFile.c_str(), paletteFile.length(), NULL, 0);
        std::wstring wPaletteFile(count, 0);
        MultiByteToWideChar(CP_ACP, 0, paletteFile.c_str(), paletteFile.length(), &wPaletteFile[0], count);

        renderPalette(wPaletteName, wPaletteFile, 300, textY + i * 80 + 50, hWnd, hdc);
    }

    int afterPaletteOffset = reader.tellg();
    reader.seekg(0, reader.end);
    int endOffset = reader.tellg();
    reader.seekg(afterPaletteOffset);
    meshEndBufLen = endOffset - afterPaletteOffset;
    if (meshEndBuf != NULL) {
        free(meshEndBuf);
    }
    meshEndBuf = (char*)malloc(meshEndBufLen);
    reader.read(meshEndBuf, meshEndBufLen);
    reader.close();
}


void updatePaletteDropdown(HWND paletteComboBox, HWND hWnd, std::wstring defaultValue, std::wstring paletteFile, int top) {
    SendMessage(paletteComboBox, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    std::vector<std::wstring> wpalettes;
    getPalettes(&wpalettes, paletteFile);

    WCHAR A[128];
    int  k = 0;

    memset(&A, 0, sizeof(A));
    int defaultIndex = 0;
    for (k = 0; k < wpalettes.size(); k += 1)
    {
        wcscpy_s(A, sizeof(A) / sizeof(WCHAR), (WCHAR*)wpalettes[k].c_str());
        if (wpalettes[k].compare(defaultValue) == 0) {
            defaultIndex = k;
        }
        // Add string to combobox.
        SendMessage(paletteComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
    }

    SendMessage(paletteComboBox, CB_SETCURSEL, (WPARAM)defaultIndex, (LPARAM)0);
    //updatePaletteDropdown(listItem, hWnd);
    SetWindowPos(paletteComboBox, NULL, 500, top, 200, 26 + 16 * 20, 0);
}

void updatePaletteFileDropdown(HWND paletteFileComboBox, HWND hWnd, std::wstring defaultValue, int top) {
    SendMessage(paletteFileComboBox, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    std::vector<std::wstring> paletteFiles;
    getPaletteFiles(&paletteFiles);

    WCHAR A[128];
    int  k = 0;

    memset(&A, 0, sizeof(A));
    int defaultIndex = 0;
    for (k = 0; k < paletteFiles.size(); k += 1)
    {
        wcscpy_s(A, sizeof(A) / sizeof(WCHAR), (WCHAR*)paletteFiles[k].c_str());
        if (paletteFiles[k].compare(defaultValue) == 0) {
            defaultIndex = k;
        }
        // Add string to combobox.
        SendMessage(paletteFileComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
    }

    SendMessage(paletteFileComboBox, CB_SETCURSEL, (WPARAM)defaultIndex, (LPARAM)0);
    SetWindowPos(paletteFileComboBox, NULL, 500, top, 200, 26 + 16 * 20, 0);
}

void updatePaletteTextDisplay(HWND hWnd) {
    int ItemIndex = SendMessage(meshCategoryComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR categoryItem[256];
    (WCHAR)SendMessage(meshCategoryComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)categoryItem);
    ItemIndex = SendMessage(meshFileComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR fileItem[256];
    (WCHAR)SendMessage(meshFileComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)fileItem);
    std::ifstream reader;
    char charBuf[12];
    int numFaces = 0;
    int numVerts = 0;
    std::wstring filePath = basePath + L"\\" + categoryItem + L"\\" + fileItem;
    reader.open(filePath.c_str(), std::ios::binary);
    reader.read(charBuf, 12);
    reader.read(charBuf, 4);
    numVerts = *(int*)charBuf;
    reader.read(charBuf, 4);
    numFaces = (*(int*)charBuf) / 3;
    int paletteOffset = 0x1F + numFaces * 6 + numVerts * 12;
    reader.seekg(paletteOffset);
    reader.read(charBuf, 1);
    int numPalettes = charBuf[0];
    int textY = 50;

    for (int i = 0; i < paletteNameComboBoxes.size(); i++) {
        DestroyWindow(paletteNameComboBoxes[i]);
    }

    for (int i = 0; i < paletteFileComboBoxes.size(); i++) {
        DestroyWindow(paletteFileComboBoxes[i]);
    }

    for (int i = 0; i < textElementHandles.size(); i++) {
        DestroyWindow(textElementHandles[i]);
    }

    paletteNameComboBoxes.clear();
    paletteFileComboBoxes.clear();
    textElementHandles.clear();

    std::string paletteName;
    std::string paletteFile;
    std::string paletteFileText;
    for (int i = 0; i < numPalettes; i++) {
        std::getline(reader, paletteName, '\0');
        int count = MultiByteToWideChar(CP_ACP, 0, paletteName.c_str(), paletteName.length(), NULL, 0);
        std::wstring wPaletteName(count, 0);
        MultiByteToWideChar(CP_ACP, 0, paletteName.c_str(), paletteName.length(), &wPaletteName[0], count);
        HWND paletteNameText = CreateWindow(WC_STATIC, wPaletteName.c_str(), WS_VISIBLE | WS_CHILD | SS_LEFT,
            300, textY + i * 80, 200, 25, hWnd, (HMENU)(i * 4 + 345), (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        HWND paletteNameSelect = CreateWindow(WC_COMBOBOX, wPaletteName.c_str(), CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
            500, textY + i * 80, 200, 25, hWnd, (HMENU)(i * 4 + 346), (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        textElementHandles.push_back(paletteNameText);
        std::string newPaletteFile;
        std::getline(reader, newPaletteFile, '\0');
        if (!newPaletteFile.empty()) {
            paletteFile = newPaletteFile;
        }
        paletteFileText = paletteFile;
        count = MultiByteToWideChar(CP_ACP, 0, paletteFileText.c_str(), paletteFileText.length(), NULL, 0);
        std::wstring wPaletteFile(count, 0);
        MultiByteToWideChar(CP_ACP, 0, paletteFileText.c_str(), paletteFileText.length(), &wPaletteFile[0], count);
        HWND paletteFileText = CreateWindow(WC_STATIC, wPaletteFile.c_str(), WS_VISIBLE | WS_CHILD | SS_LEFT,
            300, textY + i * 80 + 25, 200, 25, hWnd, (HMENU)(i * 4 + 347), (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        HWND paletteFileSelect = CreateWindow(WC_COMBOBOX, wPaletteFile.c_str(), CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
            500, textY + i * 80 + 25, 200, 25, hWnd, (HMENU)(i * 4 + 348), (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        updatePaletteDropdown(paletteNameSelect, hWnd, wPaletteName, wPaletteFile, textY + i * 80);
        updatePaletteFileDropdown(paletteFileSelect, hWnd, wPaletteFile, textY + i * 80 + 25);
        paletteNameComboBoxes.push_back(paletteNameSelect);
        paletteFileComboBoxes.push_back(paletteFileSelect);
        textElementHandles.push_back(paletteFileText);
    }
}

void saveMeshFile() {
    int ItemIndex = SendMessage(meshCategoryComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR categoryItem[256];
    (WCHAR)SendMessage(meshCategoryComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)categoryItem);
    ItemIndex = SendMessage(meshFileComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR fileItem[256];
    (WCHAR)SendMessage(meshFileComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)fileItem);
    std::ofstream writer;
    char charBuf[12];
    int numFaces = 0;
    int numVerts = 0;
    char zeroByte = 0;
    std::wstring filePath = basePath + L"\\" + categoryItem + L"\\" + fileItem;
    writer.open(filePath.c_str(), std::ios::binary);
    writer.write(meshStartBuf, meshStartBufLen);
    for (int i = 0; i < paletteFileComboBoxes.size(); i++) {
        ItemIndex = SendMessage(paletteNameComboBoxes.at(i), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        WCHAR wPaletteName[256];
        (WCHAR)SendMessage(paletteNameComboBoxes.at(i), (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)wPaletteName);
        std::wstring wPaletteNameStr = wPaletteName;
        int count = WideCharToMultiByte(CP_ACP, 0, wPaletteNameStr.c_str(), wPaletteNameStr.length(), NULL, 0, NULL, NULL);
        std::string paletteName(count, 0);
        WideCharToMultiByte(CP_ACP, 0, wPaletteNameStr.c_str(), wPaletteNameStr.length(), &paletteName[0], count, NULL, NULL);
        writer.write(paletteName.c_str(), paletteName.size());
        writer.write(&zeroByte, 1);

        ItemIndex = SendMessage(paletteFileComboBoxes.at(i), (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        WCHAR wPaletteFile[256];
        (WCHAR)SendMessage(paletteFileComboBoxes.at(i), (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)wPaletteFile);
        std::wstring wPaletteFileStr = wPaletteFile;
        count = WideCharToMultiByte(CP_ACP, 0, wPaletteFileStr.c_str(), wPaletteFileStr.length(), NULL, 0, NULL, NULL);
        std::string paletteFile(count, 0);
        WideCharToMultiByte(CP_ACP, 0, wPaletteFileStr.c_str(), wPaletteFileStr.length(), &paletteFile[0], count, NULL, NULL);
        writer.write(paletteFile.c_str(), paletteFile.size());
        writer.write(&zeroByte, 1);
    }
    writer.write(meshEndBuf, meshEndBufLen);
    writer.close();
}

void updateFileDropdown(std::wstring meshDir, HWND hWnd) {
    SendMessage(meshFileComboBox, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    getMeshFiles(meshDir);

    WCHAR A[32];
    int  k = 0;

    memset(&A, 0, sizeof(A));
    for (k = 0; k < meshFiles.size(); k += 1)
    {
        wcscpy_s(A, sizeof(A) / sizeof(WCHAR), (WCHAR*)meshFiles[k].c_str());

        // Add string to combobox.
        SendMessage(meshFileComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
    }

    // Send the CB_SETCURSEL message to display an initial item 
    //  in the selection field  
    SendMessage(meshFileComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
    if (meshFiles.size() > 20) {
        SetWindowPos(meshFileComboBox, NULL, 0, MESH_FILE_DROPDOWN_TOP, MESH_FILE_DROPDOWN_WIDTH, 26 + 16 * 20, 0);
    }
    else {
        SetWindowPos(meshFileComboBox, NULL, 0, MESH_FILE_DROPDOWN_TOP, MESH_FILE_DROPDOWN_WIDTH, 26 + 16 * meshFiles.size(), 0);
    }
    updatePaletteTextDisplay(hWnd);
}



void updateCategoryDropdown(HWND hWnd) {
    SendMessage(meshCategoryComboBox, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    getMeshCategories();

    WCHAR A[32];
    int  k = 0;

    memset(&A, 0, sizeof(A));
    for (k = 0; k < meshCategories.size(); k += 1)
    {
        wcscpy_s(A, sizeof(A) / sizeof(WCHAR), (WCHAR*)meshCategories[k].c_str());

        // Add string to combobox.
        SendMessage(meshCategoryComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
    }

    // Send the CB_SETCURSEL message to display an initial item 
    //  in the selection field  
    SendMessage(meshCategoryComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
    int ItemIndex = SendMessage(meshCategoryComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    WCHAR listItem[256];
    (WCHAR)SendMessage(meshCategoryComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)listItem);
    updateFileDropdown(listItem, hWnd);
    SetWindowPos(meshCategoryComboBox, NULL, 0, MESH_CATEG_DROPDOWN_TOP, MESH_CATEG_DROPDOWN_WIDTH, 26 + 16 * meshCategories.size(), 0);
    
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        initInfoText1(hWnd);
        initInfoText2(hWnd);
        initInfoText3(hWnd);
        initMeshCategoryDropdown(hWnd);
        initMeshFileDropdown(hWnd);
        initOpenFolderButton(hWnd);
        updateCategoryDropdown(hWnd);
        initSaveMeshButton(hWnd);
        std::wstring titleBar = L"PennyPaletteTool -- ";
        titleBar += basePath;
        SetWindowTextW(hWnd, titleBar.c_str());
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:

        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case MESH_CATEG_COMBOBOX_ID:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                WCHAR listItem[256];
                (WCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)listItem);
                updateFileDropdown(listItem, hWnd);
                RECT windowDims{ 0,0,0,0 };
                GetClientRect(hWnd, &windowDims);
                RECT textElementRect = { 300, 50, 450, windowDims.bottom };
                InvalidateRect(hWnd, &textElementRect, true);
            }
            break;
        case MESH_FILE_COMBOBOX_ID:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                updatePaletteTextDisplay(hWnd);
                RECT windowDims{ 0,0,0,0 };
                GetClientRect(hWnd, &windowDims);
                RECT textElementRect = { 300, 50, 450, windowDims.bottom };
                InvalidateRect(hWnd, &textElementRect, true);
            }
            break;
        case OPEN_ID:
        {
            PWSTR filePath = loadFolder();
            InvalidateRect(hWnd, NULL, true);
            if (filePath != NULL) {
                basePath = filePath;
                std::wstring titleBar = L"PennyPaletteTool -- ";
                titleBar += basePath;
                SetWindowTextW(hWnd, titleBar.c_str());
                updateCategoryDropdown(hWnd);
            }
            break;
        }
        case SAVE_ID:
        {
            saveMeshFile();
            RECT windowDims{ 0,0,0,0 };
            GetClientRect(hWnd, &windowDims);
            RECT textElementRect = { 300, 50, 450, windowDims.bottom };
            InvalidateRect(hWnd, &textElementRect, true);
            break;
        }
        default:
            if ((wmId >= 346) && ((wmId - 346) % 4 == 0)) {
                RECT windowDims{ 0,0,0,0 };
                GetClientRect(hWnd, &windowDims);
                RECT textElementRect = { 500, 50, 650, windowDims.bottom };
                InvalidateRect(hWnd, &textElementRect, true);

            }
            if ((wmId >= 348) && ((wmId - 348) % 4 == 0)) {
                int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                WCHAR listItem[256];
                (WCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)listItem);
                HWND window = GetDlgItem(hWnd, wmId - 2);
                updatePaletteDropdown(GetDlgItem(hWnd, wmId - 2) , hWnd, listItem, listItem, 50 + ((wmId - 348) / 4) * 80);
                RECT windowDims{ 0,0,0,0 };
                GetClientRect(hWnd, &windowDims);
                RECT textElementRect = { 500, 50, 650, windowDims.bottom };
                InvalidateRect(hWnd, &textElementRect, true);

            }


            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...

        RECT windowDims{ 0,0,0,0 };
        GetClientRect(hWnd, &windowDims);
        int winWidth = windowDims.right;
        int winHeight = windowDims.bottom;
        HBRUSH fillBrush = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &windowDims, fillBrush);

        int ItemIndex = SendMessage(meshCategoryComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        WCHAR categoryItem[256];
        (WCHAR)SendMessage(meshCategoryComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)categoryItem);
        ItemIndex = SendMessage(meshFileComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        WCHAR fileItem[256];
        (WCHAR)SendMessage(meshFileComboBox, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)fileItem);
        updatePaletteColorDisplay(hWnd, hdc);
        updatePaletteColorDisplay2(hWnd, hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
