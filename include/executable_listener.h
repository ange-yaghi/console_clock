#ifndef ATG_CONSOLE_CLOCK_EXECUTABLE_LISTENER_H
#define ATG_CONSOLE_CLOCK_EXECUTABLE_LISTENER_H

#include <string>
#include <vector>

class Settings;
class ExecutableListener {
public:
    struct State {
        bool toolRunning = false;
        bool toolFocused = false;
        std::string toolName;

        bool executableRunning = false;
        bool executableFocused = false;
    };

public:
    ExecutableListener();
    ~ExecutableListener();

    bool updateState(Settings *settings, State *state);
    bool applicationRunning(std::string_view executableName);
};

#endif /* ATG_CONSOLE_CLOCK_EXECUTABLE_LISTENER_H */
