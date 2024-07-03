#include <Windows.h>

#include <filesystem>

int main() {
    char modulePathBuffer[1024];
    GetModuleFileName(NULL, modulePathBuffer, 1024);
    std::filesystem::path modulePath(modulePathBuffer);

    HINSTANCE result = ShellExecute(NULL, "open",
                                    modulePath.parent_path()
                                            .append("ConsoleClock.exe")
                                            .string()
                                            .c_str(),
                                    NULL, NULL, SW_SHOWNORMAL);
    return 0;
}
