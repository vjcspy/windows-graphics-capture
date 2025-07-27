#pragma once

#ifdef SCREENCAPTUREDLL_EXPORTS
#define SCREENCAPTUREDLL_API __declspec(dllexport)
#else
#define SCREENCAPTUREDLL_API __declspec(dllimport)
#endif

extern "C" {
    
    // Error codes (same as core library)
    typedef enum {
        SC_SUCCESS = 0,
        SC_INITIALIZATION_FAILED = 1,
        SC_CAPTURE_ITEM_CREATION_FAILED = 2,
        SC_CAPTURE_SESSION_FAILED = 3,
        SC_TEXTURE_PROCESSING_FAILED = 4,
        SC_FILE_SAVE_FAILED = 5,
        SC_TIMEOUT_ERROR = 6,
        SC_INVALID_PARAMETER = 97,
        SC_NOT_IMPLEMENTED = 98,
        SC_UNKNOWN_ERROR = 99
    } ScreenCaptureResult;

    // Main capture function
    // outputPath: Full path to output PNG file (must be null-terminated wide string)
    // Returns: ScreenCaptureResult error code
    SCREENCAPTUREDLL_API ScreenCaptureResult CaptureScreen(const wchar_t* outputPath);

    // Capture with options
    // outputPath: Full path to output PNG file
    // hideBorder: Try to hide capture border (true recommended)
    // hideCursor: Hide mouse cursor in capture (true recommended)
    // Returns: ScreenCaptureResult error code
    SCREENCAPTUREDLL_API ScreenCaptureResult CaptureScreenWithOptions(const wchar_t* outputPath, int hideBorder, int hideCursor);

    // Get error description for a given error code
    // errorCode: The error code returned by CaptureScreen
    // Returns: Pointer to null-terminated wide string describing the error
    SCREENCAPTUREDLL_API const wchar_t* GetErrorDescription(ScreenCaptureResult errorCode);

    // Get version information
    // Returns: Pointer to null-terminated wide string with version info
    SCREENCAPTUREDLL_API const wchar_t* GetLibraryVersion();
}
