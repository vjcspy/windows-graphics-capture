#include "pch.h"
#include <windows.graphics.directx.direct3d11.interop.h> // For CreateDirect3D11DeviceFromDXGIDevice

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

// Helper function to create a D3D11 device
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
        nullptr,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Driver Type
        0,                          // Software Rasterizer (not used)
        creationFlags,              // Flags
        featureLevels,              // Feature Levels
        ARRAYSIZE(featureLevels),   // Num Feature Levels
        D3D11_SDK_VERSION,          // SDK Version
        device.put(),               // Device out
        nullptr,                    // Feature Level out
        context.put()               // Context out
    );

    if (FAILED(hr))
    {
        throw hresult_error(hr, L"Failed to create D3D11 device");
    }

    return device;
}

// Helper function to create a Direct3D11Device from an ID3D11Device
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

// Main capture logic using Windows Graphics Capture API
IAsyncAction CaptureScreenAsync()
{
    try
    {
        std::wcout << L"Initializing capture system..." << std::endl;

        // 1. Create D3D11 Device
        auto d3d11Device = CreateD3DDevice();
        auto direct3DDevice = CreateDirect3DDeviceFromD3D11Device(d3d11Device);

        // 2. Create capture item for primary monitor
        auto captureItem = CreateCaptureItemForMonitor();
        
        std::wcout << L"Capture item created. Size: " << captureItem.Size().Width << L"x" << captureItem.Size().Height << std::endl;

        // 3. Create Direct3D11CaptureFramePool
        auto framePool = Direct3D11CaptureFramePool::Create(
            direct3DDevice,
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            1,
            captureItem.Size()
        );

        // 4. Create capture session
        auto session = framePool.CreateCaptureSession(captureItem);

        // 5. Setup frame processing with simpler approach
        bool frameReceived = false;
        std::mutex frameMutex;
        std::condition_variable frameCondition;

        std::wcout << L"Setting up frame handler..." << std::endl;

        framePool.FrameArrived([&](auto const& sender, auto const& args)
        {
            std::wcout << L"FrameArrived event triggered!" << std::endl;
            
            auto frame = sender.TryGetNextFrame();
            if (frame)
            {
                try
                {
                    std::wcout << L"Frame captured! Processing..." << std::endl;

                    // Get the Direct3D11 surface
                    auto surface = frame.Surface();
                    
                    // Get the underlying D3D11 texture using interop
                    com_ptr<ID3D11Texture2D> texture;
                    auto dxgiInterfaceAccess = surface.as<::Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
                    winrt::check_hresult(dxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&texture)));

                    // Get texture description
                    D3D11_TEXTURE2D_DESC desc;
                    texture->GetDesc(&desc);

                    std::wcout << L"Texture size: " << desc.Width << L"x" << desc.Height << std::endl;

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

                    // Simple synchronous save
                    try
                    {
                        auto folder = KnownFolders::PicturesLibrary();
                        auto file = folder.CreateFileAsync(L"screen_capture.png", CreationCollisionOption::ReplaceExisting).get();

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

                        std::wcout << L"Screenshot saved successfully to Pictures\\screen_capture.png" << std::endl;
                    }
                    catch (...)
                    {
                        std::wcout << L"Error saving screenshot" << std::endl;
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
                    std::wcout << L"Error processing frame: " << ex.message().c_str() << std::endl;
                    {
                        std::lock_guard<std::mutex> lock(frameMutex);
                        frameReceived = true;
                    }
                    frameCondition.notify_one();
                }
            }
            else
            {
                std::wcout << L"No frame available" << std::endl;
            }
        });

        // 6. Start capture
        std::wcout << L"Starting capture session..." << std::endl;
        session.StartCapture();

        // Wait for frame with timeout and message pumping
        std::wcout << L"Waiting for frame (timeout: 10 seconds)..." << std::endl;
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
                std::wcout << L"Frame received and processed!" << std::endl;
            }
            else
            {
                std::wcout << L"Timeout: No frame received within 10 seconds" << std::endl;
            }
        }

        // Cleanup
        session.Close();
        framePool.Close();

        std::wcout << L"Capture completed!" << std::endl;
    }
    catch (hresult_error const& ex)
    {
        std::wcout << L"Capture error: " << ex.message().c_str() << std::endl;
    }
    catch (std::exception const& ex)
    {
        std::wcout << L"Standard error: " << std::wstring(ex.what(), ex.what() + strlen(ex.what())).c_str() << std::endl;
    }
    
    co_return;
}

int wmain()
{
    try
    {
        init_apartment(apartment_type::single_threaded);
        std::wcout << L"Windows Graphics Capture API Demo" << std::endl;
        std::wcout << L"Capturing primary monitor..." << std::endl;
        
        CaptureScreenAsync().get();
        
        std::wcout << L"Demo completed!" << std::endl;
    }
    catch (...)
    {
        std::wcout << L"Fatal error occurred" << std::endl;
        return -1;
    }
    
    return 0;
}
