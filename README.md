# ScreenCaptureApp - Advanced Windows Graphics Capture Solution

A comprehensive C++ screen capture solution with **silent border-free capturing**, flexible console interface, and seamless C# integration using Windows Graphics Capture API.

## 🌟 Key Features

- **🔇 Silent Mode**: Completely invisible screen capture without console windows or borders
- **🚫 Border-Free Capture**: Advanced options to hide Windows capture borders and cursor
- **⚡ Dual Interface**: Both standalone console app and DLL for integration
- **🎯 C# Ready**: Complete P/Invoke wrapper with full API coverage
- **💾 Direct Memory Capture**: High-performance memory-to-memory screenshot transfer (NEW!)
- **📁 Flexible Options**: Granular control over capture behavior (file or memory output)
- **📱 Modern API**: Windows Graphics Capture API with DirectX 11 acceleration

## � Project Structure

```
ScreenCaptureApp/
├── src/
│   ├── core/                   # Core capture logic (Static Library)
│   │   ├── ScreenCaptureCore.h # Main capture interface & logger classes
│   │   └── ScreenCaptureCore.cpp # Implementation with border control
│   ├── console/                # Console application
│   │   └── main.cpp           # CLI with silent/verbose modes & options
│   └── dll/                    # DLL wrapper for C# integration
│       ├── ScreenCaptureDLL.h  # C-style API exports
│       ├── ScreenCaptureDLL.cpp # DLL implementation
│       └── ScreenCaptureDLL.def # Export definitions
├── examples/
│   └── csharp/                 # C# integration examples
│       ├── Program.cs          # Demo with border comparison
│       ├── ScreenCapture.cs    # P/Invoke wrapper class
│       └── ScreenCaptureExample.csproj
├── build/                  # Build output directory
│   └── bin/Release/           # Ready-to-use executables & DLL
├── pch.h / pch.cpp            # Precompiled headers
├── CMakeLists.txt             # Build configuration
├── BUILD_GUIDE.md             # Detailed build instructions
└── README.md                  # This file
```

## 🚀 Quick Start

### Prerequisites
- **Windows 10 version 1903+** or **Windows 11**
- **Visual Studio 2019/2022** with C++ workload
- **CMake 3.14+**
- **.NET 8.0** (for C# examples)

### Build All Components
```bash
# Clone/navigate to project
cd ScreenCaptureApp

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# All components built to: build/bin/Release/
```

## 💻 Console Application Usage

### Silent Mode (Recommended)
```bash
# Clean capture without console window or borders
ScreenCaptureApp.exe "C:\screenshot.png"

# Exit codes: 0 = Success, 1+ = Error codes
```

### Advanced Options
```bash
# Verbose mode with detailed logging
ScreenCaptureApp.exe --verbose "screenshot.png"

# Keep capture border visible (for comparison)
ScreenCaptureApp.exe --show-border "with_border.png"

# Keep mouse cursor visible
ScreenCaptureApp.exe --show-cursor "with_cursor.png"

# Combine options
ScreenCaptureApp.exe --verbose --show-border --show-cursor "full_visible.png"

# Help
ScreenCaptureApp.exe --help
```

### Output Examples
```
# Silent mode (no output)
> ScreenCaptureApp.exe "test.png"
> echo $LASTEXITCODE
0

# Verbose mode
> ScreenCaptureApp.exe --verbose "test.png"
[INFO] ScreenCapture Console Application
[INFO] Hide border: Yes
[INFO] Hide cursor: Yes
[INFO] Capture item created. Size: 2560x1600
[INFO] Screenshot saved successfully to test.png
[INFO] Screenshot captured successfully!
```

## 🔗 C# Integration

### Basic File Capture
```csharp
using ScreenCaptureExample;

// Simple clean capture (hides border & cursor)
var result = ScreenCapture.Capture(@"C:\screenshot.png");

if (result == ScreenCapture.ErrorCode.Success)
{
    Console.WriteLine("Screenshot saved successfully!");
}
else
{
    Console.WriteLine($"Error: {ScreenCapture.GetErrorDescription(result)}");
}
```

### Memory Capture (High Performance)
```csharp
[DllImport("ScreenCaptureDLL.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern int CaptureScreenToMemory(bool hideBorder, bool hideCursor, out IntPtr buffer, out int size);

[DllImport("ScreenCaptureDLL.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern void FreeBuffer(IntPtr buffer);

// Direct memory capture - no file I/O
IntPtr buffer = IntPtr.Zero;
try
{
    int result = CaptureScreenToMemory(true, true, out buffer, out int size);
    if (result == 0) // Success
    {
        byte[] imageData = new byte[size];
        Marshal.Copy(buffer, imageData, 0, size);
        // Use imageData directly (PNG format)
        Console.WriteLine($"Captured {size} bytes to memory");
    }
}
finally
{
    if (buffer != IntPtr.Zero)
        FreeBuffer(buffer);
}
```

### Advanced Usage with Options
```csharp
// Full control over capture behavior
var result = ScreenCapture.Capture(
    outputPath: @"C:\screenshot.png",
    hideBorder: true,    // Hide Windows capture border
    hideCursor: true     // Hide mouse cursor
);

// Get library information
string version = ScreenCapture.GetVersion();
Console.WriteLine($"Using: {version}");

// Handle different error scenarios
switch (result)
{
    case ScreenCapture.ErrorCode.Success:
        Console.WriteLine("Perfect capture!");
        break;
    case ScreenCapture.ErrorCode.TimeoutError:
        Console.WriteLine("May need administrator privileges");
        break;
    case ScreenCapture.ErrorCode.FileSaveFailed:
        Console.WriteLine("Check output directory permissions");
        break;
    default:
        Console.WriteLine($"Error: {ScreenCapture.GetErrorDescription(result)}");
        break;
}
```

### C# Project Setup
1. **Copy DLL**: `ScreenCaptureDLL.dll` to your project output directory
2. **Add Reference**: Include `ScreenCapture.cs` in your project
3. **Set Platform**: Ensure x64 target platform

```xml
<PropertyGroup>
    <TargetFramework>net8.0-windows</TargetFramework>
    <PlatformTarget>x64</PlatformTarget>
</PropertyGroup>
```

## 📊 Capture Comparison

| Mode | Border | Cursor | Use Case |
|------|---------|---------|-----------|
| **Default** | Hidden | Hidden | **Production screenshots** |
| `--show-border` | Visible | Hidden | Development/debugging |
| `--show-cursor` | Hidden | Visible | UI demonstrations |
| Both flags | Visible | Visible | Full context capture |

## � Error Handling

### Exit Codes (Console App)
```
0  - Success
1  - Initialization failed  
2  - Capture item creation failed
3  - Capture session failed
4  - Texture processing failed
5  - File save failed
6  - Timeout error (may need admin privileges)
97 - Invalid parameters
99 - Unknown error
```

### C# Error Codes
```csharp
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
    UnknownError = 99
}
```

## 🚨 Troubleshooting

### "Timeout: No frame received within 10 seconds"
- **Run as Administrator** (most common solution)
- Check **Windows Privacy Settings** → Screenshots
- Ensure **Desktop Window Manager** is enabled
- Verify **DirectX 11** support

### "Failed to create D3D11 device"  
- Update **graphics drivers**
- Install **Visual C++ Redistributable 2019/2022**
- Try without debug layer in release builds

### C# "DLL not found" Error
- Ensure `ScreenCaptureDLL.dll` is in **same directory** as .exe
- Verify **x64 platform** consistency
- Check **dependencies** with Dependency Walker

### Console Window Appears in Silent Mode
- Don't use `--verbose` flag for silent operation
- Ensure not running from existing console window
- Check that **console allocation** is working properly

## 🎯 Advanced Features

### Border Control Technology
- **`IsBorderRequired(false)`**: Disables Windows capture border
- **`IsCursorCaptureEnabled(false)`**: Hides mouse cursor
- **Smart fallback**: Graceful handling if newer APIs unavailable
- **Message pumping**: Proper Windows event handling

### Performance Characteristics
- **Capture time**: ~100-500ms (resolution dependent)
- **Memory usage**: ~4 bytes per pixel (BGRA format)
- **GPU acceleration**: DirectX 11 hardware acceleration
- **File size**: PNG compression (typically 200-500KB for 1080p)

### Security & Privacy
- **No background service**: Runs only when called
- **No network access**: Purely local operation  
- **Minimal privileges**: Works without admin (admin helps with timeout issues)
- **Clean capture**: No system UI artifacts when border hidden

## 📚 API Reference

### Console Application
```bash
ScreenCaptureApp.exe [OPTIONS] <output_path>

OPTIONS:
  --verbose       Show detailed console output
  --show-border   Keep Windows capture border visible  
  --show-cursor   Keep mouse cursor in capture
  --help         Show usage information

EXAMPLES:
  ScreenCaptureApp.exe "screenshot.png"                    # Clean silent capture
  ScreenCaptureApp.exe --verbose "debug.png"               # With logging
  ScreenCaptureApp.exe --show-border "comparison.png"      # Border visible
```

### C# API (Legacy File-Based)
```csharp
// Core methods
ScreenCapture.Capture(string outputPath)
ScreenCapture.Capture(string outputPath, bool hideBorder, bool hideCursor)

// Utility methods  
ScreenCapture.GetErrorDescription(ErrorCode errorCode)
ScreenCapture.GetVersion()

// All methods return ErrorCode enum for consistent error handling
```

### DLL API (High Performance Memory Capture)
```csharp
// P/Invoke declarations
[DllImport("ScreenCaptureDLL.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern int CaptureScreenToMemory(bool hideBorder, bool hideCursor, out IntPtr buffer, out int size);

[DllImport("ScreenCaptureDLL.dll", CallingConvention = CallingConvention.Cdecl)]
private static extern void FreeBuffer(IntPtr buffer);

// Memory capture function
// Returns: 0 = Success, 1 = Initialization Error, 2 = Capture Error, 3 = Save Error
// Output: buffer contains PNG data, size is data length in bytes
// Important: Always call FreeBuffer(buffer) to avoid memory leaks
```

## 🔄 Integration Examples

### Automation Script (PowerShell)
```powershell
# Automated screenshot with error handling
$result = & "ScreenCaptureApp.exe" "daily_screenshot.png"
if ($LASTEXITCODE -eq 0) {
    Write-Host "Screenshot captured successfully"
} else {
    Write-Error "Capture failed with code: $LASTEXITCODE"
}
```

### C# Service Integration
```csharp
public class ScreenshotService
{
    public async Task<bool> CaptureScreenAsync(string outputPath)
    {
        return await Task.Run(() => 
        {
            var result = ScreenCapture.Capture(outputPath, hideBorder: true, hideCursor: true);
            return result == ScreenCapture.ErrorCode.Success;
        });
    }
}
```

## � Deployment

### Standalone Console App
- Copy `ScreenCaptureApp.exe`
- No additional dependencies for basic usage
- Include **Visual C++ Redistributable** for broader compatibility

### C# Application Bundle  
- Copy `ScreenCaptureDLL.dll` to application directory
- Include `ScreenCapture.cs` in your project
- Ensure **x64 platform** targeting
- Test on target machines without development tools

### System Requirements
- **OS**: Windows 10 version 1903+ or Windows 11
- **Graphics**: DirectX 11 compatible graphics card
- **Runtime**: Visual C++ Redistributable 2019/2022 (x64)
- **Memory**: Minimal (scales with screen resolution)

## 🤝 Contributing

1. Fork the repository
2. Create feature branch: `git checkout -b feature/amazing-feature`
3. Make changes with proper testing
4. Update documentation if needed
5. Submit pull request with clear description

## 📄 License

This project is provided as educational and demonstration code. Feel free to use and modify according to your needs.

## 🙏 Acknowledgments

- **Microsoft Windows Team** for Graphics Capture API
- **C++/WinRT Team** for excellent Windows Runtime bindings  
- **DirectX Team** for robust graphics infrastructure
- **Community** for feedback and testing

---

**🎯 Perfect for**: Automated testing, documentation tools, monitoring systems, desktop applications requiring clean screenshots without system artifacts.
