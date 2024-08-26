# Console Clock
A tool for tracking development time.
## How do I build it?
Note: this project currently only builds on windows
#### Step 1 - Clone the repository
If you do not have git installed, you need to install it from [here](https://git-scm.com/downloads).
In CMD use the following command: `git clone https://github.com/ange-yaghi/console_clock.git`
#### Step 2 - Install CMake
Intall the latest version of CMake [here](https://cmake.org/download/) if not already installed.
#### Step 3 - build it
From the root directory of the project, run the following command
`mkdir build `
`cd build`
`cmake ..`
`cmake --build .`
The build output will be located in the directory: `/workspace/builds/console_clock`
#### Step 4 - create the configuration file
To use console clock, you have to copy the `console_clock` folder into the root of your project. You also need to create a folder named `clock` in the project root directory. In the `clock` folder, create a text file named `lc.txt`. The contents of that file will configure console clock to run with your project.

Here is an example `lc.txt`
`-Directories`
`src`
`include`

`-Extensions`
`.h`
`.c`
`.hpp`
`.cpp`

`-Executables`
`your-application-name.exe`

`-Tools`
`devenv.exe`
`blender.exe`

`-Keywords`
`your-project-name`

Under `-Directories` you specify where your source files are.
Under `-Extensions` you specify the extensions your source files use. (for example .c for a C source file)
Under `-Executables` you specify the output executable of your project.
Under `-Tools` you speficy the tools you are using. (for example devenv.exe for Visual Studio)
Under `-Keywords` you specify your project name

#### Step 5 - making Console Clock launch when you build your application (optional)
If you're building your project using CMake or batch scripts, you can launch the program every time you build your project.

To do that, you need to create a batch script that will launch the application. Due to the nature of Windows batch scripts you need to use a run helper to not hang the build process. Here is an example batch script that will launch the application:
`cd path/to/console_clock/bit`
`ConsoleClockRun.exe`
This script can then be executed via CMake, or directly from another batch scripts. Here is a CMake example
`add_custom_target(q_start_clock ALL`
`        COMMENT "Starting console clock"`
`        COMMAND "${PROJECT_SOURCE_DIR}/console_clock/bin/run.bat"`
`)`