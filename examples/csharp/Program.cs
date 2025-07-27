using System;
using System.IO;

namespace ScreenCaptureExample
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("ScreenCapture C# Example");
            Console.WriteLine("========================");
            Console.WriteLine();

            // Display version information
            try
            {
                string version = ScreenCapture.GetVersion();
                Console.WriteLine($"Library Version: {version}");
                Console.WriteLine();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error getting version: {ex.Message}");
                Console.WriteLine("Make sure ScreenCaptureDLL.dll is in the same directory.");
                Console.WriteLine();
            }

            // Set up output path
            string outputPath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.Desktop),
                $"screenshot_{DateTime.Now:yyyyMMdd_HHmmss}.png"
            );

            Console.WriteLine($"Capturing screen to: {outputPath}");
            Console.WriteLine("Please wait...");

            try
            {
                // Test 1: Clean capture (hide border and cursor)
                Console.WriteLine("=== Test 1: Clean Capture (Hide Border & Cursor) ===");
                var result1 = ScreenCapture.Capture(outputPath + "_clean.png", true, true);
                PrintResult(result1, outputPath + "_clean.png");

                // Test 2: Capture with border visible (for comparison)
                Console.WriteLine("\n=== Test 2: With Border Visible ===");
                var result2 = ScreenCapture.Capture(outputPath.Replace(".png", "_with_border.png"), false, true);
                PrintResult(result2, outputPath.Replace(".png", "_with_border.png"));

                // Test 3: Default capture (should be clean)
                Console.WriteLine("\n=== Test 3: Default Capture ===");
                var result3 = ScreenCapture.Capture(outputPath.Replace(".png", "_default.png"));
                PrintResult(result3, outputPath.Replace(".png", "_default.png"));
            }
            catch (Exception ex)
            {
                Console.WriteLine($"✗ Exception occurred: {ex.Message}");
                
                if (ex.InnerException != null)
                {
                    Console.WriteLine($"  Inner exception: {ex.InnerException.Message}");
                }
            }

            Console.WriteLine();
            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }

        static void PrintResult(ScreenCapture.ErrorCode result, string filePath)
        {
            if (result == ScreenCapture.ErrorCode.Success)
            {
                Console.WriteLine("✓ Screenshot captured successfully!");
                
                if (File.Exists(filePath))
                {
                    var fileInfo = new FileInfo(filePath);
                    Console.WriteLine($"  File size: {fileInfo.Length / 1024:N0} KB");
                    Console.WriteLine($"  Location: {filePath}");
                }
            }
            else
            {
                Console.WriteLine($"✗ Capture failed: {result}");
                Console.WriteLine($"  Description: {ScreenCapture.GetErrorDescription(result)}");
            }
        }
    }
}
