#include "../include/executable_listener.h"

#include "../include/settings.h"

#include <Windows.h>

#include <TlHelp32.h>

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

void enumerateProcess(DWORD processId, ApplicationContext *context) {
    HANDLE process = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);
    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(process, &te)) {
        do {
            if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                                     sizeof(te.th32OwnerProcessID)) {
                if (te.th32OwnerProcessID == processId) {
                    ::EnumThreadWindows(te.th32ThreadID, ::EnumThreadWndProc,
                                        reinterpret_cast<LPARAM>(context));
                }
            }
            te.dwSize = sizeof(te);
        } while (Thread32Next(process, &te));
    }
    CloseHandle(process);
}

ExecutableListener::ExecutableListener() {}
ExecutableListener::~ExecutableListener() {}

bool ExecutableListener::updateState(Settings *settings, State *state) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(snapshot, &entry) == TRUE) {
        do {
            if (settings->executables().count(entry.szExeFile) > 0) {
                state->executableRunning = true;

                ApplicationContext context;
                context.settings = settings;
                enumerateProcess(entry.th32ProcessID, &context);

                if (context.focused) {
                    state->executableFocused = context.focused;
                }
            } else if (settings->tools().count(entry.szExeFile) > 0) {
                ApplicationContext context;
                context.settings = settings;
                enumerateProcess(entry.th32ProcessID, &context);

                if (!state->toolRunning && context.running) {
                    state->toolRunning = true;
                    state->toolName = entry.szExeFile;
                }

                if (!state->toolFocused &&
                    (context.focused && context.running)) {
                    state->toolFocused = true;
                    state->toolName = entry.szExeFile;
                }
            }
        } while (Process32Next(snapshot, &entry) == TRUE);
    }

    CloseHandle(snapshot);
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
