#include "../include/file_manager.h"

#include "../include/settings.h"

#include <filesystem>
#include <sstream>

LineCounter::LineCounter() {}
LineCounter::~LineCounter() {}

void LineCounter::countLines(Settings *settings) {
    reset();
    for (const std::string &dir : settings->directories()) {
        countLinesDirectory(dir, settings);
    }
}

size_t LineCounter::readLine(std::fstream &stream, char **buffer,
                             size_t bufferSize) {
    std::istream::sentry se(stream, true);
    std::streambuf *sb = stream.rdbuf();

    char *&target = *buffer;
    if (se) {
        for (size_t i = 0;; ++i) {
            if (i == bufferSize - 2) {
                char *newBuffer = new char[bufferSize * 2];
                memcpy(newBuffer, target, bufferSize);
                delete[] target;

                target = newBuffer;
                bufferSize *= 2;
            }

            const int c = sb->sbumpc();
            switch (c) {
                case '\n':
                    target[i] = '\0';
                    return bufferSize;
                case '\r':
                    target[i] = '\0';
                    if (sb->sgetc() == '\n') sb->sbumpc();
                    return bufferSize;
                case EOF:
                    target[i] = '\0';
                    if (i == 0) stream.setstate(std::ios::eofbit);
                    return bufferSize;
                default:
                    (*buffer)[i] = static_cast<char>(c);
            }
        }
    }
    return bufferSize;
}

void LineCounter::countLines(std::string_view fname, Settings *settings) {
    int rawLineCount = 0;
    int lineCount = 0;

    size_t bufferSize = 4;
    char *buffer = new char[bufferSize];

    std::fstream file(std::string(fname), std::ios::in);
    while (file.peek() != -1) {
        ++rawLineCount;

        bufferSize = readLine(file, &buffer, bufferSize);

        for (int i = 0; buffer[i] != '\0'; ++i) {
            if (buffer[i] != ' ' && buffer[i] != '\t') {
                ++lineCount;
                break;
            }
        }
    }

    m_lineCount += lineCount;
    m_rawLineCount += rawLineCount;

    delete[] buffer;
}

void LineCounter::countLinesDirectory(std::string_view directory,
                                      Settings *settings) {
    std::string path(directory);
    if (!std::filesystem::exists(std::filesystem::path(directory))) return;

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            countLinesDirectory(entry.path().string(), settings);
        } else if (entry.is_regular_file()) {
            std::filesystem::path p = entry.path().extension();
            if (settings->validExtension(p.string())) {
                countLines(entry.path().string(), settings);
            }
        }
    }
}
