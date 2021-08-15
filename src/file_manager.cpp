#include "../include/file_manager.h"

#include <sstream>
#include <filesystem>

void read_settings(lc_settings *settings, const char *fname) {
    std::fstream f(fname, std::ios::in); 
    if (!f.is_open()) {
        settings->directories = { "include/", "src/" };
        settings->file_extensions = { ".h", ".cpp" };

        return;
    }

    enum class mode {
        directory,
        extension
    };

    mode current_mode = mode::directory;

    std::string line;
    while (std::getline(f, line)) {
        if (line == "-Directories") {
            current_mode = mode::directory;
        }
        else if (line == "-Extensions") {
            current_mode = mode::extension;
        }
        else {
            std::stringstream ss(line);
            std::string arg;

            ss >> arg;
            if (arg.empty()) continue;

            if (current_mode == mode::directory) {
                settings->directories.push_back(arg);        
            }
            else if (current_mode == mode::extension) {
                settings->file_extensions.push_back(arg);
            }
        }
    }

    f.close();
}

bool check_extension(lc_settings *settings, const std::string &ext) {
    for (const std::string &ref : settings->file_extensions) {
        if (ref == ext) return true;
    }

    return false;
}

size_t read_line(std::istream &stream, char **buffer, size_t buffer_size) {
    std::istream::sentry se(stream, true);
    std::streambuf *sb = stream.rdbuf();

    char *&target = *buffer;

    if (se) {
        for (size_t i = 0;; ++i) {
            if (i == buffer_size - 2) {
                char *new_buffer = new char [buffer_size * 2];
                memcpy(new_buffer, target, buffer_size);
                delete[] target;

                target = new_buffer;
                buffer_size *= 2;
            }

            const int c = sb->sbumpc();
            switch(c) {
                case '\n':
                    target[i] = '\0';
                    return buffer_size;
                case '\r':
                    target[i] = '\0';
                    if (sb->sgetc() == '\n') sb->sbumpc();
                    return buffer_size;
                case EOF:
                    target[i] = '\0';
                    if (i == 0) stream.setstate(std::ios::eofbit);
                    return buffer_size;
                default:
                    (*buffer)[i] = static_cast<char>(c);
            }
        }
    }
    else {
        return buffer_size;
    }
}

void init_line_count(line_count *lc) {
    lc->line_count = 0;
    lc->raw_line_count = 0;
}

void count_lines(line_count *lc, const char *fname) {
    std::fstream file(fname, std::ios::in);
    
    int raw_line_count = 0;
    int line_count = 0;

    size_t buffer_size = 4;
    char *buffer = new char[buffer_size];

    while (file.peek() != -1) {
        ++raw_line_count;

        buffer_size = read_line(file, &buffer, buffer_size);

        for (int i = 0; buffer[i] != '\0'; ++i) {
            if (buffer[i] != ' ' && buffer[i] != '\t') {
                ++line_count;
                break;
            }
        }
    }

    delete[] buffer;

    file.close();

    lc->line_count += line_count;
    lc->raw_line_count += raw_line_count;
}

void count_lines_dir(line_count *lc, lc_settings *settings, const char *directory) {
    std::string path = directory;
    if (!std::filesystem::exists(std::filesystem::path(directory))) return;

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            count_lines_dir(lc, settings, entry.path().string().c_str());
        }
        else if (entry.is_regular_file()) {
            std::filesystem::path p = entry.path().extension();
            if (check_extension(settings, p.string().c_str())) {
                count_lines(lc, entry.path().string().c_str());
            }
        }
    }
}

void count_all(line_count *lc, lc_settings *settings) {
    init_line_count(lc);
    for (const std::string &dir : settings->directories) {
        count_lines_dir(lc, settings, dir.c_str());
    }
}
