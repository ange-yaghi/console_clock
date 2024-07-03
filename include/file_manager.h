#ifndef ATG_CONSOLE_CLOCK_FILE_MANAGER_H
#define ATG_CONSOLE_CLOCK_FILE_MANAGER_H

#include <fstream>
#include <string>
#include <vector>

class Settings;
class LineCounter {
public:
    LineCounter();
    ~LineCounter();

    void reset() { m_rawLineCount = m_lineCount = 0; }
    void countLines(Settings *settings);

    inline int rawLineCount() const { return m_rawLineCount; }
    inline int lineCount() const { return m_lineCount; }

private:
    size_t readLine(std::fstream &stream, char **buffer, size_t bufferSize);
    void countLines(std::string_view fname, Settings *settings);
    void countLinesDirectory(std::string_view directory, Settings *settings);

private:
    int m_rawLineCount = 0;
    int m_lineCount = 0;
};

#endif /* ATG_CONSOLE_CLOCK_FILE_MANAGER_H */
