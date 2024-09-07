#include <windows.h>
#include <commdlg.h>
#include <fstream>
#include <vector>
#include <string>

const std::vector<unsigned char> MAGIC_NUMBER = {
    0xD0, 0xDF, 0xFF, 0xDF, 0xDC, 0xBC, 0xBB, 0xDF,
    0xDF, 0xDB, 0xFA, 0xBB, 0xDB, 0xFD, 0xBF, 0xFD
};
const std::wstring EXTENSION = L".ics";

// Function to prompt for a new file name
std::wstring PromptForNewFileName(HWND hwnd) {
    int result = MessageBox(hwnd, L"Do you want to change the output file name to 'hkie-monthly-report.ics'?",
        L"Change File Name", MB_YESNO | MB_ICONQUESTION);
    if (result == IDYES) {
        return L"hkie-monthly-report.ics"; // Return the new file name
    }
    return L""; // Return empty string if they don't want to change
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit;
    static HWND hInsertButton;
    static HWND hRemoveButton;
    static HWND hHelpButton;
    static HWND hStatusLabel; // Static control for status message

    switch (uMsg) {
    case WM_CREATE: {
        hEdit = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            20, 20, 400, 30, hwnd, NULL, NULL, NULL);
        hInsertButton = CreateWindow(L"BUTTON", L"Append .ics", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 70, 120, 40, hwnd, (HMENU)1, NULL, NULL);
        hRemoveButton = CreateWindow(L"BUTTON", L"Remove .ics", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            160, 70, 120, 40, hwnd, (HMENU)2, NULL, NULL);
        hHelpButton = CreateWindow(L"BUTTON", L"Help", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            300, 70, 80, 40, hwnd, (HMENU)3, NULL, NULL);

        // Create a static control for status messages, centered
        hStatusLabel = CreateWindow(L"STATIC", L"Press the Insert button.", WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 120, 400, 30, hwnd, NULL, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == 1) { // Insert button
            SetWindowText(hStatusLabel, L"Processing..."); // Update status
            OPENFILENAME ofn;
            wchar_t szFile[260];
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = L'\0';
            ofn.lpstrFilter = L"All Files\0*.*\0";
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrTitle = L"Select a file";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                // Check if the file has the .ics extension
                std::wstring filePath(szFile);
                if (filePath.size() >= EXTENSION.size() &&
                    filePath.compare(filePath.size() - EXTENSION.size(), EXTENSION.size(), EXTENSION) == 0) {
                    // File already has .ics extension
                    MessageBox(hwnd, L"The file cannot be processed because it already has the .ics extension.", L"Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
                    return 0;
                }

                std::ifstream inputFile(szFile, std::ios::binary);
                if (!inputFile) {
                    MessageBox(hwnd, L"Error opening file for reading.", L"Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
                    return 0;
                }

                // Read the file data
                std::vector<char> fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
                inputFile.close();

                // Write to the new file with the .ics extension
                std::wstring outputFileName = std::wstring(szFile) + EXTENSION;
                std::wstring newFileName = PromptForNewFileName(hwnd);

                if (!newFileName.empty()) {
                    outputFileName = newFileName; // Use the new file name if provided
                }

                std::ofstream outputFile(outputFileName, std::ios::binary);
                if (!outputFile) {
                    MessageBox(hwnd, L"Error opening file for writing.", L"Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
                    return 0;
                }

                // Write the magic number
                outputFile.write(reinterpret_cast<const char*>(MAGIC_NUMBER.data()), MAGIC_NUMBER.size());
                // Write the original file data
                outputFile.write(fileData.data(), fileData.size());
                outputFile.close();

                SetWindowText(hStatusLabel, L"Done."); // Update status to done
                MessageBox(hwnd, L"Magic number inserted successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
            }
            else {
                SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
            }
        }
        else if (LOWORD(wParam) == 2) { // Remove button
            SetWindowText(hStatusLabel, L"Processing..."); // Update status
            OPENFILENAME ofn;
            wchar_t szFile[260];
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = L'\0';
            ofn.lpstrFilter = L"All Files\0*.*\0";
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrTitle = L"Select a file";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn)) {
                std::ifstream inputFile(szFile, std::ios::binary);
                if (!inputFile) {
                    MessageBox(hwnd, L"Error opening file for reading.", L"Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
                    return 0;
                }

                // Read the magic number
                std::vector<unsigned char> magicBuffer(MAGIC_NUMBER.size());
                inputFile.read(reinterpret_cast<char*>(magicBuffer.data()), MAGIC_NUMBER.size());

                if (magicBuffer != MAGIC_NUMBER) {
                    MessageBox(hwnd, L"Magic number not found in the file.", L"Error", MB_OK | MB_ICONERROR);
                    inputFile.close();
                    SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
                    return 0;
                }

                // Read the remaining file data (after the magic number)
                std::vector<char> fileData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
                inputFile.close();

                // Derive the original filename by removing the .ics extension
                std::wstring originalFilename = std::wstring(szFile).substr(0, std::wstring(szFile).size() - EXTENSION.size());

                // Write the original data back to the original file
                std::ofstream outputFile(originalFilename, std::ios::binary);
                if (!outputFile) {
                    MessageBox(hwnd, L"Error opening file for writing.", L"Error", MB_OK | MB_ICONERROR);
                    SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
                    return 0;
                }

                // Write the original file data back
                outputFile.write(fileData.data(), fileData.size());
                outputFile.close();

                SetWindowText(hStatusLabel, L"Done."); // Update status to done
                MessageBox(hwnd, L"Magic number removed successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
            }
            else {
                SetWindowText(hStatusLabel, L"Press the Insert button."); // Reset status
            }
        }
        else if (LOWORD(wParam) == 3) { // Help button
            MessageBox(hwnd, L"Extension Converter \nAuthor: Ethan C.\nVersion: 1.1", L"About", MB_OK | MB_ICONINFORMATION);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    const wchar_t CLASS_NAME[] = L"MagicNumberApp";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Extension Converter", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 480, 200, NULL, NULL, hInstance, NULL); // Increased size

    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}