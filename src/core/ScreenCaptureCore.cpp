#include "ScreenCaptureCore.h"
#include "../../pch.h"
#include <windows.graphics.directx.direct3d11.interop.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <filesystem>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

namespace ScreenCaptureCore
{
    // Console logger implementation
    void ConsoleLogger::LogInfo(const std::wstring& message)
    {
        std::wcout << L"[INFO] " << message << std::endl;
    }

    void ConsoleLogger::LogError(const std::wstring& message)
    {
        std::wcerr << L"[ERROR] " << message << std::endl;
    }

    // Helper function to create D3D11 device
    com_ptr<ID3D11Device> CreateD3DDevice()
    {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        com_ptr<ID3D11Device> device;
        com_ptr<ID3D11DeviceContext> context;
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            device.put(),
            nullptr,
            context.put()
        );

        if (FAILED(hr))
        {
            throw hresult_error(hr, L"Failed to create D3D11 device");
        }

        return device;
    }

    // Helper function to create Direct3D11Device from ID3D11Device
    IDirect3DDevice CreateDirect3DDeviceFromD3D11Device(const com_ptr<ID3D11Device>& d3d11Device)
    {
        com_ptr<IDXGIDevice> dxgiDevice;
        winrt::check_hresult(d3d11Device->QueryInterface(dxgiDevice.put()));

        winrt::com_ptr<winrt::Windows::Foundation::IInspectable> inspectable;
        winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), reinterpret_cast<::IInspectable**>(inspectable.put())));

        return inspectable.as<IDirect3DDevice>();
    }

    // Helper function to create GraphicsCaptureItem for primary monitor
    GraphicsCaptureItem CreateCaptureItemForMonitor()
    {
        auto factory = winrt::get_activation_factory<GraphicsCaptureItem>();
        auto interop = factory.as<IGraphicsCaptureItemInterop>();

        // Get primary monitor
        HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

        GraphicsCaptureItem item{ nullptr };
        winrt::check_hresult(interop->CreateForMonitor(
            primaryMonitor,
            winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
            winrt::put_abi(item)
        ));

        return item;
    }

    // ScreenCapture implementation
    ScreenCapture::ScreenCapture(ILogger* logger)
        : m_logger(logger ? logger : &m_defaultLogger)
    {
        try
        {
            init_apartment(apartment_type::single_threaded);
        }
        catch (...)
        {
            // Apartment may already be initialized
        }
    }

    ScreenCapture::~ScreenCapture()
    {
    }

    void ScreenCapture::Log(const std::wstring& message)
    {
        if (m_logger)
        {
            m_logger->LogInfo(message);
        }
    }

    void ScreenCapture::LogError(const std::wstring& message)
    {
        if (m_logger)
        {
            m_logger->LogError(message);
        }
    }

    ErrorCode ScreenCapture::CaptureToFile(const std::wstring& outputPath)
    {
        // Default: hide border and cursor for cleaner capture
        return CaptureToFile(outputPath, true, true);
    }

    ErrorCode ScreenCapture::CaptureToFile(const std::wstring& outputPath, bool hideBorder, bool hideCursor)
    {
        return InternalCapture(outputPath, hideBorder, hideCursor);
    }

    ErrorCode ScreenCapture::InternalCapture(const std::wstring& outputPath, bool hideBorder, bool hideCursor)
    {
        try
        {
            Log(L"Initializing capture system...");

            // 1. Create D3D11 Device
            auto d3d11Device = CreateD3DDevice();
            auto direct3DDevice = CreateDirect3DDeviceFromD3D11Device(d3d11Device);

            // 2. Create capture item for primary monitor
            auto captureItem = CreateCaptureItemForMonitor();
            Log(L"Capture item created. Size: " + std::to_wstring(captureItem.Size().Width) + L"x" + std::to_wstring(captureItem.Size().Height));

            // 3. Create Direct3D11CaptureFramePool
            auto framePool = Direct3D11CaptureFramePool::Create(
                direct3DDevice,
                DirectXPixelFormat::B8G8R8A8UIntNormalized,
                1,
                captureItem.Size()
            );

            // 4. Create capture session
            auto session = framePool.CreateCaptureSession(captureItem);

            // 5. Configure capture session options
            if (hideCursor)
            {
                Log(L"Disabling cursor in capture...");
                session.IsCursorCaptureEnabled(false);
            }

            if (hideBorder)
            {
                Log(L"Attempting to disable border...");
                try
                {
                    session.IsBorderRequired(false);
                }
                catch (...)
                {
                    Log(L"Warning: Could not disable border (may require newer Windows version)");
                }
            }

            // 6. Setup frame processing
            bool frameReceived = false;
            bool captureSuccess = false;
            std::mutex frameMutex;
            std::condition_variable frameCondition;

            Log(L"Setting up frame handler...");

            framePool.FrameArrived([&](auto const& sender, auto const& args)
            {
                Log(L"FrameArrived event triggered!");

                auto frame = sender.TryGetNextFrame();
                if (frame)
                {
                    try
                    {
                        Log(L"Frame captured! Processing...");

                        // Get the Direct3D11 surface
                        auto surface = frame.Surface();

                        // Get the underlying D3D11 texture using interop
                        com_ptr<ID3D11Texture2D> texture;
                        auto dxgiInterfaceAccess = surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
                        winrt::check_hresult(dxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&texture)));

                        // Get texture description
                        D3D11_TEXTURE2D_DESC desc;
                        texture->GetDesc(&desc);

                        Log(L"Texture size: " + std::to_wstring(desc.Width) + L"x" + std::to_wstring(desc.Height));

                        // Create staging texture for CPU access
                        desc.Usage = D3D11_USAGE_STAGING;
                        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                        desc.BindFlags = 0;
                        desc.MiscFlags = 0;

                        com_ptr<ID3D11Texture2D> stagingTexture;
                        winrt::check_hresult(d3d11Device->CreateTexture2D(&desc, nullptr, stagingTexture.put()));

                        // Copy to staging texture
                        com_ptr<ID3D11DeviceContext> context;
                        d3d11Device->GetImmediateContext(context.put());
                        context->CopyResource(stagingTexture.get(), texture.get());

                        // Map the texture
                        D3D11_MAPPED_SUBRESOURCE mappedResource;
                        winrt::check_hresult(context->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mappedResource));

                        // Create bitmap data
                        uint32_t dataSize = mappedResource.RowPitch * desc.Height;
                        std::vector<uint8_t> bitmapData(dataSize);
                        memcpy(bitmapData.data(), mappedResource.pData, dataSize);

                        context->Unmap(stagingTexture.get(), 0);

                        // Save to file
                        try
                        {
                            // Get folder and filename from path
                            std::filesystem::path filePath(outputPath);
                            auto parentPath = filePath.parent_path();
                            auto fileName = filePath.filename().wstring();
                            
                            // If no parent path specified, use current directory
                            if (parentPath.empty())
                            {
                                parentPath = std::filesystem::current_path();
                            }
                            
                            // Ensure directory exists
                            if (!std::filesystem::exists(parentPath))
                            {
                                std::filesystem::create_directories(parentPath);
                            }
                            
                            // Create file using Windows Storage API
                            auto folder = StorageFolder::GetFolderFromPathAsync(parentPath.wstring()).get();
                            auto file = folder.CreateFileAsync(fileName, CreationCollisionOption::ReplaceExisting).get();

                            InMemoryRandomAccessStream stream;
                            BitmapEncoder encoder = BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), stream).get();

                            encoder.SetPixelData(
                                BitmapPixelFormat::Bgra8,
                                BitmapAlphaMode::Ignore,
                                desc.Width,
                                desc.Height,
                                96.0,
                                96.0,
                                bitmapData
                            );

                            encoder.FlushAsync().get();

                            auto outputStream = file.OpenAsync(FileAccessMode::ReadWrite).get();
                            stream.Seek(0);
                            RandomAccessStream::CopyAsync(stream, outputStream).get();
                            outputStream.FlushAsync().get();
                            outputStream.Close();

                            Log(L"Screenshot saved successfully to " + outputPath);
                            captureSuccess = true;
                        }
                        catch (...)
                        {
                            LogError(L"Error saving screenshot to file");
                        }

                        // Signal completion
                        {
                            std::lock_guard<std::mutex> lock(frameMutex);
                            frameReceived = true;
                        }
                        frameCondition.notify_one();
                    }
                    catch (hresult_error const& ex)
                    {
                        LogError(L"Error processing frame: " + std::wstring(ex.message()));
                        {
                            std::lock_guard<std::mutex> lock(frameMutex);
                            frameReceived = true;
                        }
                        frameCondition.notify_one();
                    }
                }
                else
                {
                    LogError(L"No frame available");
                }
            });

            // 7. Start capture
            Log(L"Starting capture session...");
            session.StartCapture();

            // Wait for frame with timeout and message pumping
            Log(L"Waiting for frame (timeout: 10 seconds)...");
            {
                std::unique_lock<std::mutex> lock(frameMutex);
                auto start = std::chrono::steady_clock::now();
                auto timeout = std::chrono::seconds(10);

                while (!frameReceived && (std::chrono::steady_clock::now() - start) < timeout)
                {
                    lock.unlock();

                    // Pump messages to allow Windows events to be processed
                    MSG msg;
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }

                    // Small sleep to prevent busy waiting
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));

                    lock.lock();
                }

                if (frameReceived)
                {
                    Log(L"Frame received and processed!");
                }
                else
                {
                    LogError(L"Timeout: No frame received within 10 seconds");
                    return ErrorCode::TimeoutError;
                }
            }

            // Cleanup
            session.Close();
            framePool.Close();

            Log(L"Capture completed!");

            return captureSuccess ? ErrorCode::Success : ErrorCode::FileSaveFailed;
        }
        catch (hresult_error const& ex)
        {
            LogError(L"Capture error: " + std::wstring(ex.message()));
            return ErrorCode::CaptureSessionFailed;
        }
        catch (std::exception const& ex)
        {
            std::wstring errorMsg(ex.what(), ex.what() + strlen(ex.what()));
            LogError(L"Standard error: " + errorMsg);
            return ErrorCode::UnknownError;
        }
        catch (...)
        {
            LogError(L"Unknown error occurred");
            return ErrorCode::UnknownError;
        }
    }
}
