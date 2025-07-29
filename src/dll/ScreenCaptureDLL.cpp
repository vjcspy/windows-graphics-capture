#include "ScreenCaptureDLL.h"
#include "../../pch.h"
#include "../core/ScreenCaptureCore.h"
#include <string>
#include <memory>

using namespace ScreenCaptureCore;

// Convert core error codes to DLL error codes
ScreenCaptureResult ConvertErrorCode(ErrorCode coreError)
{
    switch (coreError)
    {
    case ErrorCode::Success:
        return SC_SUCCESS;
    case ErrorCode::InitializationFailed:
        return SC_INITIALIZATION_FAILED;
    case ErrorCode::CaptureItemCreationFailed:
        return SC_CAPTURE_ITEM_CREATION_FAILED;
    case ErrorCode::CaptureSessionFailed:
        return SC_CAPTURE_SESSION_FAILED;
    case ErrorCode::TextureProcessingFailed:
        return SC_TEXTURE_PROCESSING_FAILED;
    case ErrorCode::FileSaveFailed:
        return SC_FILE_SAVE_FAILED;
    case ErrorCode::TimeoutError:
        return SC_TIMEOUT_ERROR;
    case ErrorCode::UnknownError:
    default:
        return SC_UNKNOWN_ERROR;
    }
}

extern "C" {

    SCREENCAPTUREDLL_API ScreenCaptureResult CaptureScreen(const wchar_t* outputPath)
    {
        // Default: hide border and cursor
        return CaptureScreenWithOptions(outputPath, 1, 1);
    }

    SCREENCAPTUREDLL_API ScreenCaptureResult CaptureScreenWithOptions(const wchar_t* outputPath, int hideBorder, int hideCursor)
    {
        // Validate input parameters
        if (!outputPath || wcslen(outputPath) == 0)
        {
            return SC_INVALID_PARAMETER;
        }

        try
        {
            // Create silent logger for DLL (no console output)
            SilentLogger logger;
            
            // Create screen capture instance
            ScreenCapture capture(&logger);

            // Perform capture with options
            auto result = capture.CaptureToFile(std::wstring(outputPath), hideBorder != 0, hideCursor != 0);

            // Convert and return result
            return ConvertErrorCode(result);
        }
        catch (...)
        {
            return SC_UNKNOWN_ERROR;
        }
    }

    SCREENCAPTUREDLL_API ScreenCaptureResult CaptureScreenToMemory(unsigned char** outputBuffer, unsigned int* bufferSize, int hideBorder, int hideCursor)
    {
        // Validate input parameters
        if (!outputBuffer || !bufferSize)
        {
            return SC_INVALID_PARAMETER;
        }

        try
        {
            // Create silent logger for DLL (no console output)
            SilentLogger logger;
            
            // Create screen capture instance
            ScreenCapture capture(&logger);

            // Capture to memory buffer
            std::vector<uint8_t> buffer;
            auto result = capture.CaptureToMemory(buffer, hideBorder != 0, hideCursor != 0);

            if (result == ErrorCode::Success && !buffer.empty())
            {
                // Allocate buffer for caller
                *bufferSize = static_cast<unsigned int>(buffer.size());
                *outputBuffer = static_cast<unsigned char*>(malloc(*bufferSize));
                
                if (*outputBuffer)
                {
                    memcpy(*outputBuffer, buffer.data(), *bufferSize);
                    return SC_SUCCESS;
                }
                else
                {
                    return SC_UNKNOWN_ERROR; // Memory allocation failed
                }
            }
            else
            {
                *outputBuffer = nullptr;
                *bufferSize = 0;
                return ConvertErrorCode(result);
            }
        }
        catch (...)
        {
            *outputBuffer = nullptr;
            *bufferSize = 0;
            return SC_UNKNOWN_ERROR;
        }
    }

    SCREENCAPTUREDLL_API void FreeBuffer(unsigned char* buffer)
    {
        if (buffer)
        {
            free(buffer);
        }
    }

    SCREENCAPTUREDLL_API const wchar_t* GetErrorDescription(ScreenCaptureResult errorCode)
    {
        switch (errorCode)
        {
        case SC_SUCCESS:
            return L"Operation completed successfully";
        case SC_INITIALIZATION_FAILED:
            return L"Failed to initialize capture system";
        case SC_CAPTURE_ITEM_CREATION_FAILED:
            return L"Failed to create capture item for monitor";
        case SC_CAPTURE_SESSION_FAILED:
            return L"Failed to start capture session";
        case SC_TEXTURE_PROCESSING_FAILED:
            return L"Failed to process captured texture";
        case SC_FILE_SAVE_FAILED:
            return L"Failed to save screenshot to file";
        case SC_TIMEOUT_ERROR:
            return L"Timeout waiting for frame capture";
        case SC_INVALID_PARAMETER:
            return L"Invalid parameter provided";
        case SC_NOT_IMPLEMENTED:
            return L"Feature not yet implemented";
        case SC_UNKNOWN_ERROR:
        default:
            return L"Unknown error occurred";
        }
    }

    SCREENCAPTUREDLL_API const wchar_t* GetLibraryVersion()
    {
        return L"ScreenCaptureDLL v1.0.0 - Windows Graphics Capture API";
    }

} // extern "C"

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize COM/WinRT when DLL is loaded
        try
        {
            winrt::init_apartment(winrt::apartment_type::single_threaded);
        }
        catch (...)
        {
            // May already be initialized
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // Cleanup if needed
        break;
    }
    return TRUE;
}
