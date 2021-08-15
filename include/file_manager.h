#ifndef CONSOLE_CLOCK_FILE_MANAGER_H 
#define CONSOLE_CLOCK_FILE_MANAGER_H

#include <string>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct line_count {
    int raw_line_count;
    int line_count;
};

struct lc_settings {
    std::vector<std::string> file_extensions;
    std::vector<std::string> directories;
};

void read_settings(lc_settings *settings, const char *fname);
bool check_extension(lc_settings *settings, const std::string &ext);

size_t read_line(std::istream &stream, char **buffer, size_t buffer_size); 
void init_line_count(line_count *lc);
void count_lines(line_count *lc, const char *fname);
void count_lines_dir(line_count *lc, lc_settings *settings, const char *directory);
void count_all(line_count *lc, lc_settings *settings);

#endif /* CONSOLE_CLOCK_FILE_MANAGER_H */
