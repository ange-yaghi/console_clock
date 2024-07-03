#ifndef ATG_CONSOLE_CLOCK_SETTINGS_H
#define ATG_CONSOLE_CLOCK_SETTINGS_H

#include <set>
#include <string>
#include <vector>

class Settings {
public:
    Settings();
    ~Settings();

    void readFromFile(std::string_view fname, std::string_view root);
    bool validExtension(std::string_view ext);
    inline const auto &directories() const { return m_directories; }
    inline const auto &executables() const { return m_executables; }
    inline const auto &tools() const { return m_tools; }
    inline const auto &keywords() const { return m_keywords; }

private:
    std::set<std::string> m_fileExtensions;
    std::vector<std::string> m_directories;
    std::set<std::string> m_executables;
    std::set<std::string> m_tools;
    std::set<std::string> m_keywords;
};

#endif /* ATG_CONSOLE_CLOCK_SETTINGS_H */
