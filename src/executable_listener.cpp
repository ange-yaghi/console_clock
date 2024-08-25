#include "../include/executable_listener.h"

#include "../include/settings.h"

#include <Windows.h>

#include <TlHelp32.h>

#include <map>
#include <string>

struct ApplicationContext {
    Settings *settings = nullptr;
    bool running = false;
    bool focused = false;
};

BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lParam) {
    ApplicationContext *context =
            reinterpret_cast<ApplicationContext *>(lParam);

    char windowTitleBuffer[1024];
    ::GetWindowText(hwnd, windowTitleBuffer, 1024);

    char test[1024];
    ::GetWindowText(GetForegroundWindow(), test, 1024);

    if (GetForegroundWindow() == hwnd) { context->focused = true; }

    std::string windowTitle(windowTitleBuffer);
    for (const auto &keyword : context->settings->keywords()) {
        if (windowTitle.find(keyword) != std::string::npos) {
            context->running = true;
        }
    }
    return TRUE;
}

ExecutableListener::ExecutableListener() {}
ExecutableListener::~ExecutableListener() {}

bool ExecutableListener::updateState(Settings *settings, State *state) {
    std::map<DWORD, std::string> executables, tools;
    std::map<DWORD, std::vector<DWORD>> threads;

    HANDLE snapshot =
            CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);

    {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot, &entry) == TRUE) {
            do {
                if (settings->executables().count(entry.szExeFile) > 0) {
                    executables.emplace(entry.th32ProcessID, entry.szExeFile);
                    state->executableRunning = true;
                } else if (settings->tools().count(entry.szExeFile) > 0) {
                    tools.emplace(entry.th32ProcessID, entry.szExeFile);
                }
            } while (Process32Next(snapshot, &entry) == TRUE);
        }
    }

    for (auto &process : executables) {
        threads.emplace(process.first, std::vector<DWORD>());
    }

    for (auto &process : tools) {
        threads.emplace(process.first, std::vector<DWORD>());
    }

    {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(snapshot, &te) == TRUE) {
            do {
                if (te.dwSize >=
                    FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                            sizeof(te.th32OwnerProcessID)) {
                    if (threads.count(te.th32OwnerProcessID) > 0) {
                        threads[te.th32OwnerProcessID].push_back(
                                te.th32ThreadID);
                    }
                }
                te.dwSize = sizeof(te);
            } while (Thread32Next(snapshot, &te) == TRUE);
        }
    }

    CloseHandle(snapshot);

    for (auto &process : threads) {
        for (auto &thread : process.second) {
            ApplicationContext context;
            context.settings = settings;
            ::EnumThreadWindows(thread, ::EnumThreadWndProc,
                                reinterpret_cast<LPARAM>(&context));

            if (executables.count(process.first) > 0) {
                if (context.focused) {
                    state->executableFocused = context.focused;
                }
            } else if (tools.count(process.first) > 0) {
                if (!state->toolRunning && context.running) {
                    state->toolRunning = true;
                    state->toolName = tools[process.first];
                }

                if (!state->toolFocused &&
                    (context.focused && context.running)) {
                    state->toolFocused = true;
                    state->toolName = tools[process.first];
                }
            }
        }
    }

    return false;
}

bool ExecutableListener::applicationRunning(std::string_view executableName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    bool result = false;
    if (Process32First(snapshot, &entry) == TRUE) {
        do {
            if (executableName.compare(entry.szExeFile) == 0 &&
                entry.th32ProcessID != GetCurrentProcessId()) {
                result = true;
            }
        } while (Process32Next(snapshot, &entry) == TRUE);
    }

    CloseHandle(snapshot);
    return result;
}
