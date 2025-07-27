# C# Integration Examples

## Complete C# Usage Examples

### 1. Basic Screenshot Service
```csharp
using System;
using System.IO;
using System.Threading.Tasks;
using ScreenCaptureExample;

public class ScreenshotService
{
    private readonly string _outputDirectory;

    public ScreenshotService(string outputDirectory = null)
    {
        _outputDirectory = outputDirectory ?? 
            Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyPictures), "Screenshots");
        
        Directory.CreateDirectory(_outputDirectory);
    }

    /// <summary>
    /// Takes a clean screenshot (no border, no cursor)
    /// </summary>
    public async Task<string> TakeScreenshotAsync(string fileName = null)
    {
        fileName ??= $"screenshot_{DateTime.Now:yyyyMMdd_HHmmss}.png";
        string fullPath = Path.Combine(_outputDirectory, fileName);

        var result = await Task.Run(() => ScreenCapture.Capture(fullPath));
        
        if (result == ScreenCapture.ErrorCode.Success)
        {
            return fullPath;
        }
        
        throw new InvalidOperationException($"Screenshot failed: {ScreenCapture.GetErrorDescription(result)}");
    }

    /// <summary>
    /// Takes screenshot with custom options
    /// </summary>
    public async Task<string> TakeScreenshotAsync(string fileName, bool hideBorder, bool hideCursor)
    {
        string fullPath = Path.Combine(_outputDirectory, fileName);

        var result = await Task.Run(() => ScreenCapture.Capture(fullPath, hideBorder, hideCursor));
        
        if (result == ScreenCapture.ErrorCode.Success)
        {
            return fullPath;
        }
        
        throw new InvalidOperationException($"Screenshot failed: {ScreenCapture.GetErrorDescription(result)}");
    }
}
```

### 2. Console Application with Error Handling
```csharp
using System;
using System.IO;
using ScreenCaptureExample;

class Program
{
    static async Task<int> Main(string[] args)
    {
        try
        {
            var service = new ScreenshotService();
            
            Console.WriteLine("ScreenCapture C# Integration Demo");
            Console.WriteLine($"Library Version: {ScreenCapture.GetVersion()}");
            Console.WriteLine();

            // Test 1: Basic screenshot
            Console.WriteLine("Taking basic screenshot...");
            string path1 = await service.TakeScreenshotAsync();
            Console.WriteLine($"✓ Saved to: {path1}");

            // Test 2: Screenshot with border for comparison
            Console.WriteLine("Taking screenshot with border...");
            string path2 = await service.TakeScreenshotAsync("with_border.png", false, true);
            Console.WriteLine($"✓ Saved to: {path2}");

            Console.WriteLine("All screenshots completed successfully!");
            return 0;
        }
        catch (InvalidOperationException ex)
        {
            Console.WriteLine($"❌ Error: {ex.Message}");
            return 1;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"❌ Unexpected error: {ex.Message}");
            return 2;
        }
    }
}
```

### 3. WPF Integration Example
```csharp
using System;
using System.IO;
using System.Windows;
using System.Windows.Media.Imaging;
using ScreenCaptureExample;

public partial class MainWindow : Window
{
    private readonly ScreenshotService _screenshotService;

    public MainWindow()
    {
        InitializeComponent();
        _screenshotService = new ScreenshotService();
    }

    private async void TakeScreenshot_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            statusLabel.Content = "Taking screenshot...";
            
            string filePath = await _screenshotService.TakeScreenshotAsync();
            
            // Display in UI
            var bitmap = new BitmapImage(new Uri(filePath));
            screenshotImage.Source = bitmap;
            
            statusLabel.Content = $"Screenshot saved: {Path.GetFileName(filePath)}";
        }
        catch (Exception ex)
        {
            statusLabel.Content = $"Error: {ex.Message}";
            MessageBox.Show(ex.Message, "Screenshot Error", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    private async void TakeCleanScreenshot_Click(object sender, RoutedEventArgs e)
    {
        await TakeScreenshotWithOptions(true, true, "Clean screenshot");
    }

    private async void TakeDebugScreenshot_Click(object sender, RoutedEventArgs e)
    {
        await TakeScreenshotWithOptions(false, false, "Debug screenshot");
    }

    private async Task TakeScreenshotWithOptions(bool hideBorder, bool hideCursor, string description)
    {
        try
        {
            statusLabel.Content = $"Taking {description.ToLower()}...";
            
            string fileName = $"{description.Replace(" ", "_").ToLower()}_{DateTime.Now:HHmmss}.png";
            string filePath = await _screenshotService.TakeScreenshotAsync(fileName, hideBorder, hideCursor);
            
            statusLabel.Content = $"{description} saved: {Path.GetFileName(filePath)}";
        }
        catch (Exception ex)
        {
            statusLabel.Content = $"Error: {ex.Message}";
        }
    }
}
```

### 4. Web API Integration
```csharp
using Microsoft.AspNetCore.Mvc;
using ScreenCaptureExample;

[ApiController]
[Route("api/[controller]")]
public class ScreenshotController : ControllerBase
{
    private readonly ScreenshotService _screenshotService;
    private readonly ILogger<ScreenshotController> _logger;

    public ScreenshotController(ILogger<ScreenshotController> logger)
    {
        _logger = logger;
        _screenshotService = new ScreenshotService(
            Path.Combine(Directory.GetCurrentDirectory(), "wwwroot", "screenshots")
        );
    }

    [HttpPost("capture")]
    public async Task<IActionResult> CaptureScreenshot([FromBody] ScreenshotRequest request)
    {
        try
        {
            string fileName = $"api_screenshot_{DateTime.Now:yyyyMMdd_HHmmss}.png";
            
            string filePath = await _screenshotService.TakeScreenshotAsync(
                fileName, 
                request.HideBorder ?? true, 
                request.HideCursor ?? true
            );

            string relativePath = Path.GetRelativePath(
                Path.Combine(Directory.GetCurrentDirectory(), "wwwroot"), 
                filePath
            ).Replace('\\', '/');

            _logger.LogInformation($"Screenshot captured: {fileName}");

            return Ok(new { 
                success = true, 
                fileName = fileName,
                url = $"/screenshots/{Path.GetFileName(filePath)}",
                timestamp = DateTime.UtcNow
            });
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Failed to capture screenshot");
            return BadRequest(new { success = false, error = ex.Message });
        }
    }

    [HttpGet("version")]
    public IActionResult GetVersion()
    {
        return Ok(new { 
            version = ScreenCapture.GetVersion(),
            timestamp = DateTime.UtcNow
        });
    }
}

public class ScreenshotRequest
{
    public bool? HideBorder { get; set; }
    public bool? HideCursor { get; set; }
}
```

### 5. Error Handling Best Practices
```csharp
public static class ScreenCaptureHelper
{
    public static async Task<string> SafeCaptureAsync(string outputPath, int maxRetries = 3)
    {
        for (int attempt = 1; attempt <= maxRetries; attempt++)
        {
            try
            {
                var result = await Task.Run(() => ScreenCapture.Capture(outputPath));
                
                switch (result)
                {
                    case ScreenCapture.ErrorCode.Success:
                        return outputPath;
                        
                    case ScreenCapture.ErrorCode.TimeoutError:
                        if (attempt < maxRetries)
                        {
                            Console.WriteLine($"Timeout on attempt {attempt}, retrying...");
                            await Task.Delay(1000 * attempt); // Progressive delay
                            continue;
                        }
                        throw new TimeoutException("Screenshot capture timed out. Try running as Administrator.");
                        
                    case ScreenCapture.ErrorCode.FileSaveFailed:
                        throw new UnauthorizedAccessException($"Cannot save to {outputPath}. Check permissions.");
                        
                    case ScreenCapture.ErrorCode.InitializationFailed:
                        throw new InvalidOperationException("Graphics system initialization failed. Update graphics drivers.");
                        
                    default:
                        throw new InvalidOperationException($"Capture failed: {ScreenCapture.GetErrorDescription(result)}");
                }
            }
            catch (Exception ex) when (attempt < maxRetries)
            {
                Console.WriteLine($"Attempt {attempt} failed: {ex.Message}");
                await Task.Delay(1000 * attempt);
            }
        }
        
        throw new InvalidOperationException($"Failed to capture screenshot after {maxRetries} attempts");
    }
}
```

## Deployment Configuration

### appsettings.json (for Web API)
```json
{
  "Screenshot": {
    "OutputDirectory": "wwwroot/screenshots",
    "MaxFileAge": "7.00:00:00",
    "CleanupEnabled": true,
    "DefaultHideBorder": true,
    "DefaultHideCursor": true
  }
}
```

### Dependency Injection Setup
```csharp
// Program.cs or Startup.cs
services.AddSingleton<ScreenshotService>();
services.Configure<ScreenshotOptions>(configuration.GetSection("Screenshot"));
```

## Installation Steps for New Project

1. **Copy Files**:
   - `ScreenCaptureDLL.dll` → Your output directory
   - `ScreenCapture.cs` → Your project

2. **Update Project File**:
   ```xml
   <PropertyGroup>
     <TargetFramework>net8.0-windows</TargetFramework>
     <PlatformTarget>x64</PlatformTarget>
   </PropertyGroup>
   ```

3. **Test Integration**:
   ```csharp
   var result = ScreenCapture.Capture("test.png");
   Console.WriteLine($"Result: {result}");
   ```

4. **Handle Dependencies**:
   - Ensure Visual C++ Redistributable 2019/2022 (x64) is installed
   - Verify Windows 10 version 1903+ or Windows 11

This covers all major C# integration scenarios!
