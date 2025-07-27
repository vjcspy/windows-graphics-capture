# ScreenCapture Implementation Guide

This document explains how to build and use the restructured ScreenCapture project.

## 🏗️ Project Structure

```
ScreenCaptureApp/
├── src/
│   ├── core/                   # Core capture logic (static library)
│   │   ├── ScreenCaptureCore.h
│   │   └── ScreenCaptureCore.cpp
│   ├── console/                # Console application
│   │   └── main.cpp
│   └── dll/                    # DLL wrapper for C#
│       ├── ScreenCaptureDLL.h
│       ├── ScreenCaptureDLL.cpp
│       └── ScreenCaptureDLL.def
├── examples/
│   └── csharp/                 # C# usage examples
│       ├── Program.cs
│       ├── ScreenCapture.cs
│       └── ScreenCaptureExample.csproj
├── pch.h / pch.cpp            # Precompiled headers
└── CMakeLists.txt             # Build configuration
```

## 🔧 Build Instructions

### Build C++ Components

```bash
# Navigate to project directory
cd ScreenCaptureApp

# Create and enter build directory
mkdir build
cd build

# Configure build
cmake .. -G "Visual Studio 17 2022" -A x64

# Build all targets
cmake --build . --config Release

# Or build specific targets
cmake --build . --config Release --target ScreenCaptureApp
cmake --build . --config Release --target ScreenCaptureDLL
```

### Build C# Example

```bash
# Navigate to C# example directory
cd examples/csharp

# Build the example
dotnet build -c Release

# Run the example
dotnet run
```

## 📱 Usage Examples

### 1. Console Application

#### Silent Mode (No Console Window)
```bash
# Capture to specific file - runs silently
ScreenCaptureApp.exe "C:\screenshots\capture.png"

# Exit codes:
# 0 = Success
# 1+ = Error codes (see ScreenCaptureCore.h)
```

#### Verbose Mode (With Console Output)
```bash
# Show console and detailed logging
ScreenCaptureApp.exe --verbose "C:\screenshots\capture.png"

# Help
ScreenCaptureApp.exe --help
```

### 2. C# Integration

#### Basic Usage
```csharp
using ScreenCaptureExample;

// Simple capture
var result = ScreenCapture.Capture(@"C:\temp\screenshot.png");

if (result == ScreenCapture.ErrorCode.Success)
{
    Console.WriteLine("Screenshot saved successfully!");
}
else
{
    Console.WriteLine($"Error: {ScreenCapture.GetErrorDescription(result)}");
}
```

#### Advanced Usage
```csharp
// Check library version
string version = ScreenCapture.GetVersion();
Console.WriteLine($"Using: {version}");

// Capture with error handling
try
{
    string outputPath = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
        $"screenshot_{DateTime.Now:yyyyMMdd_HHmmss}.png"
    );
    
    var result = ScreenCapture.Capture(outputPath);
    
    switch (result)
    {
        case ScreenCapture.ErrorCode.Success:
            Console.WriteLine($"Screenshot saved to: {outputPath}");
            break;
        case ScreenCapture.ErrorCode.TimeoutError:
            Console.WriteLine("Capture timed out - may need administrator privileges");
            break;
        default:
            Console.WriteLine($"Capture failed: {ScreenCapture.GetErrorDescription(result)}");
            break;
    }
}
catch (InvalidOperationException ex)
{
    Console.WriteLine($"DLL Error: {ex.Message}");
}
```

## 📦 Distribution

### For Console App
- Copy `ScreenCaptureApp.exe` from `build/bin/Release/`
- Include Visual C++ Redistributable if targeting machines without it

### For C# Integration
1. Copy `ScreenCaptureDLL.dll` from `build/bin/Release/`
2. Include `ScreenCapture.cs` in your C# project
3. Ensure DLL is in same directory as your C# executable

### Deployment Requirements
- **Windows 10 version 1903+** or **Windows 11**
- **DirectX 11** support (available on most modern systems)
- **Visual C++ Redistributable 2019/2022** (x64)

## 🔍 Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | Success | Operation completed successfully |
| 1 | InitializationFailed | Failed to initialize capture system |
| 2 | CaptureItemCreationFailed | Failed to create capture item for monitor |
| 3 | CaptureSessionFailed | Failed to start capture session |
| 4 | TextureProcessingFailed | Failed to process captured texture |
| 5 | FileSaveFailed | Failed to save screenshot to file |
| 6 | TimeoutError | Timeout waiting for frame capture |
| 97 | InvalidParameter | Invalid parameter provided |
| 99 | UnknownError | Unknown error occurred |

## 🛠️ Development Notes

### Console App Features
- **Silent mode**: No console window, suitable for automation
- **Verbose mode**: Full console output for debugging
- **Command line parsing**: Flexible argument handling
- **Exit codes**: Proper error reporting for scripts

### DLL Features
- **C-style exports**: Compatible with P/Invoke
- **Error codes**: No exceptions thrown across DLL boundary
- **Thread-safe**: Safe for multi-threaded applications
- **Memory management**: No memory leaks

### Core Library Features
- **Configurable logging**: Silent or verbose modes
- **Exception handling**: Comprehensive error recovery
- **Resource management**: RAII with proper cleanup
- **Windows Graphics Capture API**: Modern capture method

## 🚨 Troubleshooting

### "No frame received within 10 seconds"
- Run as Administrator
- Check Windows Privacy Settings → Screenshots
- Ensure Desktop Window Manager is enabled

### "Failed to create D3D11 device"
- Update graphics drivers
- Verify DirectX 11 support
- Try running without debug layer

### C# "DLL not found"
- Ensure `ScreenCaptureDLL.dll` is in same directory as executable
- Check that you're using x64 build for x64 application
- Verify Visual C++ Redistributable is installed

### Console window appears in silent mode
- Make sure you're not passing `--verbose` flag
- Check that the application is not being run from a console

## 📈 Performance Notes

- **Capture time**: ~100-500ms depending on screen resolution
- **Memory usage**: Approximately 4 bytes per pixel (BGRA)
- **File size**: PNG compression reduces size significantly
- **CPU usage**: Minimal - most processing is GPU-accelerated
