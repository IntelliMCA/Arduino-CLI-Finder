#define UNICODE
#define _UNICODE

#include <iostream>
#include <string>
#include <windows.h>
#include <ctime>

// Define all the display texts as constants
const std::wstring VERSION_TEXT = L"WinFScanner Version 0.4.1";
const std::wstring AUTHOR_TEXT = L"by mca_rz";
const std::wstring ERROR_ACCESS_DIR = L"[ERROR] Unable to access directory ";
const std::wstring TIMEOUT_ERROR = L"[TIMEOUT] Search took too long.";
const std::wstring SEARCHING_TEXT = L"\nLocating arduino-cli.exe..\n\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
const std::wstring FOUND_TEXT = L"[FOUND] File located";
const std::wstring NOT_FOUND_TEXT = L"[NOT FOUND] File not found.";

struct SearchResult
{
    bool fileFound;
    static std::wstring filepath;
};

std::wstring SearchResult::filepath = L"";

SearchResult SearchAndCountDirectories(const std::wstring &directory, const std::wstring &fileName, time_t startTime, int timeout)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    SearchResult result = {false};

    // Construct the search pattern
    std::wstring searchPattern = directory + L"\\*";

    // Start searching
    hFind = FindFirstFile(searchPattern.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        std::wcerr << ERROR_ACCESS_DIR << "(" << directory << ")" << std::endl;
        return result; // Unable to access directory
    }

    do
    {
        const std::wstring fileOrDirName = findFileData.cFileName;

        // Skip the current directory and parent directory
        if (fileOrDirName == L"." || fileOrDirName == L"..")
        {
            continue;
        }

        // Check if it's a directory
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Check for timeout
            if (difftime(time(nullptr), startTime) > timeout)
            {
                std::wcerr << TIMEOUT_ERROR << std::endl;
                FindClose(hFind); // Ensure handle is closed
                return result;    // Exit gracefully
            }

            // Recursively search directories
            SearchResult subResult = SearchAndCountDirectories(directory + L"\\" + fileOrDirName, fileName, startTime, timeout);

            if (subResult.fileFound)
            {
                FindClose(hFind); // Clean up current handle before exiting
                return subResult; // Exit recursion with file found
            }
        }
        else
        {
            // Check if the file matches the specified file name
            if (fileOrDirName == fileName)
            {
                result.fileFound = true; // File found
                result.filepath = directory;
                FindClose(hFind);        // Clean up current handle
                return result;           // Exit immediately
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    // Cleanup
    FindClose(hFind);
    return result;
}

int main()
{
    system("cls");
    std::wstring startDirectory = L"C:\\";      
    std::wstring fileName = L"arduino-cli.exe";

    // Print initial version information
    std::wcout << VERSION_TEXT << std::endl;
    std::wcout << AUTHOR_TEXT << std::endl;

    // Static search message
    std::wcout << SEARCHING_TEXT << std::endl;

    // Start the timer
    time_t startTime = time(nullptr);
    int timeout = 180; // Set timeout duration in seconds

    // Search directories
    SearchResult result = SearchAndCountDirectories(startDirectory, fileName, startTime, timeout);

    if (result.fileFound)
    {
        std::wcout << FOUND_TEXT << ": (" << result.filepath << ")" <<  std::endl;
        return 0; // Exit with success code
    }
    else
    {
        std::wcout << NOT_FOUND_TEXT << ": " << result.filepath << std::endl;
        return -1; // Exit with failure code
    }
}
