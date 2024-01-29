#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <target_value> <poll_interval_seconds>" << endl;
        return -1;
    }

    // Load target addresses from the file
    vector<DWORD_PTR> targetAddresses;
    ifstream inFile("target_addresses.txt");
    if (inFile.is_open()) {
        DWORD_PTR address;
        while (inFile >> hex >> address) {
            targetAddresses.push_back(address);
        }
        inFile.close();
    } else {
        cout << "Failed to open target_addresses.txt for reading." << endl;
        return -1;
    }

    const char* windowTitle = "";

    HWND hwnd = FindWindowA(NULL, windowTitle);

    if (hwnd == NULL) {
        cout << "Can't find process window." << endl;
        Sleep(2000);
        return -1;
    }

    DWORD procID;
    GetWindowThreadProcessId(hwnd, &procID);

    if (procID == 0) {
        cout << "Can't find process ID." << endl;
        Sleep(2000);
        return -1;
    }

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

    if (handle == NULL) {
        cout << "Failed to open process. Error: " << GetLastError() << endl;
        Sleep(2000);
        return -1;
    }

    int targetValue = atoi(argv[1]);
    int pollIntervalSeconds = atoi(argv[2]);
    bool targetFound = false;

    // Check for changes in the memory at target addresses
    while (!targetFound) {
        for (DWORD_PTR address : targetAddresses) {
            int readTest = 0;

            if (!ReadProcessMemory(handle, (PBYTE*)address, &readTest, sizeof(readTest), 0)) {
                cout << "Failed to read process memory at address 0x" << hex << address << dec << ". Error: " << GetLastError() << endl;
                continue; // Continue to the next address on error
            }

            if (readTest == targetValue) {
                cout << "Value at address 0x" << hex << address << dec << " changed to " << targetValue << "!" << endl;
                targetFound = true;
                break; // Exit the loop on target value found
            }
        }
        cout << "Cycle done" << endl;

        Sleep(pollIntervalSeconds * 1000); // Adjust the sleep duration as needed

    }

    CloseHandle(handle);

    return 0;
}
