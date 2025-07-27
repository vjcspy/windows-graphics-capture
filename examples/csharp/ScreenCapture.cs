using System;
using System.Runtime.InteropServices;

namespace ScreenCaptureExample
{
    public static class ScreenCapture
    {
        // Error codes matching the DLL
        public enum ErrorCode : int
        {
            Success = 0,
            InitializationFailed = 1,
            CaptureItemCreationFailed = 2,
            CaptureSessionFailed = 3,
            TextureProcessingFailed = 4,
            FileSaveFailed = 5,
            TimeoutError = 6,
            InvalidParameter = 97,
            NotImplemented = 98,
            UnknownError = 99
        }

        // P/Invoke declarations
        [DllImport("ScreenCaptureDLL.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern int CaptureScreen([MarshalAs(UnmanagedType.LPWStr)] string outputPath);

        [DllImport("ScreenCaptureDLL.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern int CaptureScreenWithOptions([MarshalAs(UnmanagedType.LPWStr)] string outputPath, int hideBorder, int hideCursor);

        [DllImport("ScreenCaptureDLL.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetErrorDescription(int errorCode);

        [DllImport("ScreenCaptureDLL.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetLibraryVersion();

        /// <summary>
        /// Captures the primary monitor and saves it as a PNG file (with border and cursor hidden)
        /// </summary>
        /// <param name="outputPath">Full path to the output PNG file</param>
        /// <returns>ErrorCode indicating success or failure</returns>
        public static ErrorCode Capture(string outputPath)
        {
            return Capture(outputPath, true, true);
        }

        /// <summary>
        /// Captures the primary monitor and saves it as a PNG file with options
        /// </summary>
        /// <param name="outputPath">Full path to the output PNG file</param>
        /// <param name="hideBorder">Hide the capture border (recommended: true)</param>
        /// <param name="hideCursor">Hide the mouse cursor (recommended: true)</param>
        /// <returns>ErrorCode indicating success or failure</returns>
        public static ErrorCode Capture(string outputPath, bool hideBorder, bool hideCursor)
        {
            if (string.IsNullOrEmpty(outputPath))
            {
                return ErrorCode.InvalidParameter;
            }

            try
            {
                int result = CaptureScreenWithOptions(outputPath, hideBorder ? 1 : 0, hideCursor ? 1 : 0);
                return (ErrorCode)result;
            }
            catch (DllNotFoundException)
            {
                throw new InvalidOperationException("ScreenCaptureDLL.dll not found. Make sure it's in the same directory as your application.");
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException($"Error calling screen capture: {ex.Message}", ex);
            }
        }

        /// <summary>
        /// Gets a human-readable description for an error code
        /// </summary>
        /// <param name="errorCode">The error code to describe</param>
        /// <returns>Error description string</returns>
        public static string GetErrorDescription(ErrorCode errorCode)
        {
            try
            {
                IntPtr ptr = GetErrorDescription((int)errorCode);
                return Marshal.PtrToStringUni(ptr) ?? "Unknown error";
            }
            catch
            {
                return "Failed to get error description";
            }
        }

        /// <summary>
        /// Gets the version information of the screen capture library
        /// </summary>
        /// <returns>Version string</returns>
        public static string GetVersion()
        {
            try
            {
                IntPtr ptr = GetLibraryVersion();
                return Marshal.PtrToStringUni(ptr) ?? "Unknown version";
            }
            catch
            {
                return "Failed to get version";
            }
        }
    }
}
