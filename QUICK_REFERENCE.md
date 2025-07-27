# ScreenCapture Quick Reference

## 🚀 Quick Commands

### Console App (Silent)
```bash
# Clean capture (recommended)
ScreenCaptureApp.exe "screenshot.png"

# With border visible  
ScreenCaptureApp.exe --show-border "debug.png"

# Verbose mode
ScreenCaptureApp.exe --verbose "detailed.png"
```

### C# Integration
```csharp
// Simple usage
var result = ScreenCapture.Capture(@"C:\screenshot.png");

// With options
var result = ScreenCapture.Capture(@"C:\screenshot.png", 
    hideBorder: true, hideCursor: true);

// Error handling
if (result != ScreenCapture.ErrorCode.Success) {
    Console.WriteLine(ScreenCapture.GetErrorDescription(result));
}
```

## 📋 Error Codes
| Code | Meaning | Action |
|------|---------|---------|
| 0 | Success | ✅ Perfect |
| 6 | Timeout | 🔑 Run as Admin |
| 5 | File Save Failed | 📁 Check path permissions |
| 1-4 | System/Driver | 🔧 Update drivers |

## 🎯 Best Practices
- **Use default settings** for clean captures
- **Run as Administrator** if timeout issues
- **Hide border & cursor** for production screenshots
- **Use verbose mode** for debugging
- **Check exit codes** in automation scripts

## 📦 Files You Need
- **Console**: `ScreenCaptureApp.exe` (standalone)
- **C# Integration**: `ScreenCaptureDLL.dll` + `ScreenCapture.cs`
- **Dependencies**: Visual C++ Redistributable 2019/2022 (x64)

## 🔧 Common Fixes
- **Timeout**: Run as Administrator
- **DLL not found**: Copy to same directory as .exe
- **Permission denied**: Check output folder permissions
- **Border still shows**: Update to Windows 10 1903+
