#include "../../pch.h"
#include "../core/ScreenCaptureCore.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using namespace ScreenCaptureCore;

void ShowUsage()
{
    std::wcout << L"Usage:" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe <output_path>              - Silent mode (hide border & cursor)" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe --verbose <output_path>    - Verbose mode with console output" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe --show-border <output_path> - Keep capture border visible" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe --show-cursor <output_path> - Keep mouse cursor visible" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe --help                     - Show this help" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Examples:" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe \"C:\\screenshot.png\"        - Clean capture (recommended)" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe --verbose \"D:\\capture.png\" - With detailed logs" << std::endl;
    std::wcout << L"  ScreenCaptureApp.exe --show-border \"test.png\"   - Keep border visible" << std::endl;
}

bool ParseCommandLine(int argc, wchar_t* argv[], bool& verboseMode, std::wstring& outputPath, bool& hideBorder, bool& hideCursor)
{
    if (argc < 2)
    {
        return false;
    }

    std::vector<std::wstring> args;
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }

    // Default settings - hide border and cursor for clean capture
    verboseMode = false;
    hideBorder = true;
    hideCursor = true;

    // Check for help
    if (args[0] == L"--help" || args[0] == L"-h" || args[0] == L"/?")
    {
        ShowUsage();
        return false;
    }

    // Parse arguments
    size_t outputIndex = 0;
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (args[i] == L"--verbose" || args[i] == L"-v")
        {
            verboseMode = true;
        }
        else if (args[i] == L"--show-border")
        {
            hideBorder = false;
        }
        else if (args[i] == L"--show-cursor")
        {
            hideCursor = false;
        }
        else
        {
            // This should be the output path
            outputPath = args[i];
            outputIndex = i;
            break;
        }
    }

    // Validate we have an output path
    if (outputPath.empty())
    {
        if (verboseMode)
        {
            std::wcerr << L"Error: Output path required" << std::endl;
        }
        return false;
    }

    // Validate output path
    try
    {
        std::filesystem::path path(outputPath);
        if (path.extension() != L".png")
        {
            if (verboseMode)
            {
                std::wcerr << L"Warning: Output file should have .png extension" << std::endl;
            }
        }

        // Ensure parent directory exists
        auto parentPath = path.parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath))
        {
            if (verboseMode)
            {
                std::wcout << L"Creating directory: " << parentPath.wstring() << std::endl;
            }
            std::filesystem::create_directories(parentPath);
        }
    }
    catch (const std::exception& ex)
    {
        if (verboseMode)
        {
            std::wcerr << L"Error validating output path: " << std::wstring(ex.what(), ex.what() + strlen(ex.what())) << std::endl;
        }
        return false;
    }

    return true;
}

void SetupConsole(bool verboseMode)
{
    if (verboseMode)
    {
        // Ensure console is allocated and visible
        if (!AllocConsole())
        {
            // Console may already exist, try to attach to parent
            if (!AttachConsole(ATTACH_PARENT_PROCESS))
            {
                // If that fails, allocate a new one
                AllocConsole();
            }
        }

        // Redirect stdout, stdin, stderr to console
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

        // Make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
        // point to console as well
        std::ios::sync_with_stdio(true);
        
        // Show console window
        ShowWindow(GetConsoleWindow(), SW_SHOW);
    }
    else
    {
        // Hide console window for silent mode
        HWND consoleWindow = GetConsoleWindow();
        if (consoleWindow != NULL)
        {
            ShowWindow(consoleWindow, SW_HIDE);
        }
    }
}

int wmain(int argc, wchar_t* argv[])
{
    bool verboseMode = false;
    bool hideBorder = true;
    bool hideCursor = true;
    std::wstring outputPath;

    // Parse command line
    if (!ParseCommandLine(argc, argv, verboseMode, outputPath, hideBorder, hideCursor))
    {
        if (argc >= 2 && (std::wstring(argv[1]) == L"--help" || std::wstring(argv[1]) == L"-h" || std::wstring(argv[1]) == L"/?"))
        {
            return 0; // Help was shown, exit normally
        }
        return 1; // Invalid arguments
    }

    // Setup console based on mode
    SetupConsole(verboseMode);

    try
    {
        // Create logger based on mode
        std::unique_ptr<ILogger> logger;
        if (verboseMode)
        {
            logger = std::make_unique<ConsoleLogger>();
            logger->LogInfo(L"ScreenCapture Console Application");
            logger->LogInfo(L"Verbose mode enabled");
            logger->LogInfo(L"Output path: " + outputPath);
            logger->LogInfo(L"Hide border: " + std::wstring(hideBorder ? L"Yes" : L"No"));
            logger->LogInfo(L"Hide cursor: " + std::wstring(hideCursor ? L"Yes" : L"No"));
        }
        else
        {
            logger = std::make_unique<SilentLogger>();
        }

        // Create screen capture instance
        ScreenCapture capture(logger.get());

        // Perform capture with options
        auto result = capture.CaptureToFile(outputPath, hideBorder, hideCursor);

        // Handle result
        if (result == ErrorCode::Success)
        {
            if (verboseMode)
            {
                logger->LogInfo(L"Screenshot captured successfully!");
            }
            return 0;
        }
        else
        {
            if (verboseMode)
            {
                logger->LogError(L"Capture failed with error code: " + std::to_wstring(static_cast<int>(result)));
            }
            return static_cast<int>(result);
        }
    }
    catch (...)
    {
        if (verboseMode)
        {
            std::wcerr << L"Fatal error occurred" << std::endl;
        }
        return 99; // Unknown error
    }
}
