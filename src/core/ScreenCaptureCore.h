#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace ScreenCaptureCore
{
    // Error codes
    enum class ErrorCode : int
    {
        Success = 0,
        InitializationFailed = 1,
        CaptureItemCreationFailed = 2,
        CaptureSessionFailed = 3,
        TextureProcessingFailed = 4,
        FileSaveFailed = 5,
        TimeoutError = 6,
        UnknownError = 99
    };

    // Logger interface
    class ILogger
    {
    public:
        virtual ~ILogger() = default;
        virtual void LogInfo(const std::wstring& message) = 0;
        virtual void LogError(const std::wstring& message) = 0;
    };

    // Silent logger (no output)
    class SilentLogger : public ILogger
    {
    public:
        void LogInfo(const std::wstring& message) override {}
        void LogError(const std::wstring& message) override {}
    };

    // Console logger
    class ConsoleLogger : public ILogger
    {
    public:
        void LogInfo(const std::wstring& message) override;
        void LogError(const std::wstring& message) override;
    };

    // Main screen capture class
    class ScreenCapture
    {
    public:
        ScreenCapture(ILogger* logger = nullptr);
        ~ScreenCapture();

        // Capture primary monitor and save to PNG file
        ErrorCode CaptureToFile(const std::wstring& outputPath);
        
        // Capture with options
        ErrorCode CaptureToFile(const std::wstring& outputPath, bool hideBorder, bool hideCursor = true);

        // Capture to memory buffer (PNG format)
        ErrorCode CaptureToMemory(std::vector<uint8_t>& outputBuffer, bool hideBorder = true, bool hideCursor = true);

    private:
        ILogger* m_logger;
        SilentLogger m_defaultLogger;

        void Log(const std::wstring& message);
        void LogError(const std::wstring& message);
        
        // Internal capture with options
        ErrorCode InternalCapture(const std::wstring& outputPath, bool hideBorder, bool hideCursor);
        ErrorCode InternalCaptureToMemory(std::vector<uint8_t>& outputBuffer, bool hideBorder, bool hideCursor);
    };
}
