#include <iostream>
#include <Windows.h>
#include <vector>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Custom exception handler for structured exception handling
LONG WINAPI HandleException(PEXCEPTION_POINTERS exceptionInfo) {
    cout << "Caught exception. Continuing..." << endl;
    return EXCEPTION_CONTINUE_EXECUTION;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <target_value> <duration_in_seconds>" << endl;
        return -1;
    }

    const char* windowTitle = "";

    HWND hwnd = FindWindowA(NULL, windowTitle);

    if (hwnd == NULL) {
        cout << "Can't find process window." << endl;
        return -1;
    }

    DWORD procID;
    GetWindowThreadProcessId(hwnd, &procID);

    if (procID == 0) {
        cout << "Can't find process ID." << endl;
        return -1;
    }

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

    if (handle == NULL) {
        cout << "Failed to open process. Error: " << GetLastError() << endl;
        return -1;
    }

    int targetValue = atoi(argv[1]);
    int durationInSeconds = atoi(argv[2]);

    vector<DWORD_PTR> targetAddresses;

    // Set a custom exception handler
    SetUnhandledExceptionFilter(HandleException);

    auto startTime = steady_clock::now();
    auto endTime = startTime + seconds(durationInSeconds);

    // Loop to search the entire memory for the target value within the specified duration
    for (DWORD_PTR address = 0x00000000; address < 0xFFFFFFFF; address += sizeof(int)) {
        int readTest = 0;

        // Use structured exception handling to catch access violations
        try {
            if (ReadProcessMemory(handle, (PBYTE*)address, &readTest, sizeof(readTest), 0) &&
                readTest == targetValue) {
                targetAddresses.push_back(address);
            }
        } catch (...) {
            continue; // Continue the loop on exception
        }

        // Check if the specified duration has passed
        if (steady_clock::now() >= endTime) {
            break;
        }
    }

    // Save target addresses to a file
    ofstream outFile("target_addresses.txt", ios::trunc);
    if (outFile.is_open()) {
        for (DWORD_PTR address : targetAddresses) {
            outFile << "0x" << hex << address << dec << endl;
        }
        outFile.close();
        cout << "Target addresses saved to target_addresses.txt" << endl;
    } else {
        cout << "Failed to open target_addresses.txt for writing." << endl;
    }

    CloseHandle(handle);

    return 0;
}
