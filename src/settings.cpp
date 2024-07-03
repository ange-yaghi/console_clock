#include "../include/settings.h"

#include <fstream>
#include <sstream>

Settings::Settings() {}
Settings::~Settings() {}

void Settings::readFromFile(std::string_view fname, std::string_view root_) {
    std::string root(root_);
    std::fstream f(root + std::string(fname), std::ios::in);
    if (!f.is_open()) {
        m_directories = {root + "include/", root + "src/"};
        m_fileExtensions = {".h", ".cpp"};
        return;
    }

    enum class Mode { Directory, Extension, Executable, Tools, Keywords };
    Mode currentMode = Mode::Directory;
    for (std::string line; std::getline(f, line);) {
        if (line == "-Directories") {
            currentMode = Mode::Directory;
            continue;
        } else if (line == "-Extensions") {
            currentMode = Mode::Extension;
            continue;
        } else if (line == "-Executables") {
            currentMode = Mode::Executable;
            continue;
        } else if (line == "-Tools") {
            currentMode = Mode::Tools;
            continue;
        } else if (line == "-Keywords") {
            currentMode = Mode::Keywords;
            continue;
        }

        std::string arg0, arg1;
        std::stringstream(line) >> arg0 >> arg1;
        if (arg0.empty()) { continue; }
        if (currentMode == Mode::Directory) {
            m_directories.push_back(root + arg0);
        } else if (currentMode == Mode::Extension) {
            m_fileExtensions.insert(arg0);
        } else if (currentMode == Mode::Executable) {
            m_executables.insert(arg0);
        } else if (currentMode == Mode::Tools) {
            m_tools.insert(arg0);
        } else if (currentMode == Mode::Keywords) {
            m_keywords.insert(arg0);
        }
    }
}

bool Settings::validExtension(std::string_view ext) {
    return (m_fileExtensions.count(std::string(ext)) != 0);
}
