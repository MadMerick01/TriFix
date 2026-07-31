#include "TriFix/Renderer.h"

#include <dxgi.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace {

void ThrowIfFailed(HRESULT result, const char* message) {
    if (FAILED(result)) {
        throw std::runtime_error(message);
    }
}

std::vector<char> ReadShaderBytecode(const wchar_t* filename) {
    wchar_t executablePath[MAX_PATH]{};
    if (GetModuleFileNameW(nullptr, executablePath, MAX_PATH) == 0) {
        throw std::runtime_error("Failed to locate the application executable.");
    }

    const auto shaderPath = std::filesystem::path(executablePath).parent_path() / filename;
    std::ifstream shader(shaderPath, std::ios::binary | std::ios::ate);
    if (!shader) {
        throw std::runtime_error("Failed to open compiled calibration grid shader.");
    }

    const auto byteCount = shader.tellg();
    if (byteCount <= 0) {
        throw std::runtime_error("The compiled calibration grid shader is empty.");
    }
    std::vector<char> bytecode(static_cast<std::size_t>(byteCount));
    shader.seekg(0);
    shader.read(bytecode.data(), static_cast<std::streamsize>(bytecode.size()));
    if (!shader) {
        throw std::runtime_error("Failed to read compiled calibration grid shader.");
    }
    return bytecode;
}

} // namespace

namespace trifix {

Renderer::Renderer(HWND window) : window_(window) {
    RECT clientRectangle{};
    if (GetClientRect(window_, &clientRectangle) == FALSE) {
        throw std::runtime_error("Failed to query the window dimensions.");
    }
    width_ = static_cast<UINT>(clientRectangle.right - clientRectangle.left);
    height_ = static_cast<UINT>(clientRectangle.bottom - clientRectangle.top);

    DXGI_SWAP_CHAIN_DESC swapChainDescription{};
    swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescription.SampleDesc.Count = 1;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.BufferCount = 2;
    swapChainDescription.OutputWindow = window_;
    swapChainDescription.Windowed = TRUE;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel{};
    const D3D_FEATURE_LEVEL requestedFeatureLevels[] = {D3D_FEATURE_LEVEL_11_0};
    ThrowIfFailed(
        D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            requestedFeatureLevels,
            static_cast<UINT>(std::size(requestedFeatureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDescription,
            swapChain_.GetAddressOf(),
            device_.GetAddressOf(),
            &featureLevel,
            context_.GetAddressOf()),
        "Failed to create the Direct3D 11 device and swap chain.");

    CreateRenderTarget();
    CreateCalibrationGridPipeline();
}

void Renderer::Render() {
    ResizeIfNeeded();

    constexpr float clearColor[] = {0.10F, 0.10F, 0.10F, 1.0F};
    context_->OMSetRenderTargets(1, renderTarget_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(renderTarget_.Get(), clearColor);
    DrawCalibrationGrid();
    ThrowIfFailed(swapChain_->Present(1, 0), "Failed to present a rendered frame.");
}

void Renderer::CreateCalibrationGridPipeline() {
    // A full-screen quad supplies the only geometry. Pixel-space grid generation stays in
    // the shader, keeping this pass independent from the window dimensions and ready to be
    // replaced or extended by future reprojection shader passes.
    constexpr std::array<float, 12> vertices{
        -1.0F, -1.0F, -1.0F, 1.0F, 1.0F, -1.0F,
        1.0F, -1.0F, -1.0F, 1.0F, 1.0F, 1.0F};
    D3D11_BUFFER_DESC bufferDescription{};
    bufferDescription.ByteWidth = static_cast<UINT>(sizeof(vertices));
    bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertexData{};
    vertexData.pSysMem = vertices.data();
    ThrowIfFailed(
        device_->CreateBuffer(&bufferDescription, &vertexData, gridVertexBuffer_.GetAddressOf()),
        "Failed to create the calibration grid vertex buffer.");

    const auto vertexBytecode = ReadShaderBytecode(L"GridVertex.cso");
    const auto pixelBytecode = ReadShaderBytecode(L"GridPixel.cso");
    ThrowIfFailed(device_->CreateVertexShader(vertexBytecode.data(), vertexBytecode.size(), nullptr,
                                               gridVertexShader_.GetAddressOf()),
                  "Failed to create the calibration grid vertex shader.");
    ThrowIfFailed(device_->CreatePixelShader(pixelBytecode.data(), pixelBytecode.size(), nullptr,
                                              gridPixelShader_.GetAddressOf()),
                  "Failed to create the calibration grid pixel shader.");

    constexpr D3D11_INPUT_ELEMENT_DESC inputElement{
        "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
    ThrowIfFailed(device_->CreateInputLayout(&inputElement, 1, vertexBytecode.data(),
                                              vertexBytecode.size(), gridInputLayout_.GetAddressOf()),
                  "Failed to create the calibration grid input layout.");
}

void Renderer::DrawCalibrationGrid() {
    // Configure this self-contained pass explicitly so later passes cannot accidentally
    // inherit state. The viewport tracks the current back buffer after every resize.
    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MaxDepth = 1.0F;
    context_->RSSetViewports(1, &viewport);

    constexpr UINT stride = sizeof(float) * 2;
    constexpr UINT offset = 0;
    ID3D11Buffer* vertexBuffer = gridVertexBuffer_.Get();
    context_->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context_->IASetInputLayout(gridInputLayout_.Get());
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(gridVertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(gridPixelShader_.Get(), nullptr, 0);
    context_->Draw(6, 0);
}

void Renderer::CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(
        swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
        "Failed to acquire the swap chain back buffer.");
    ThrowIfFailed(
        device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTarget_.GetAddressOf()),
        "Failed to create the render target view.");
}

void Renderer::ResizeIfNeeded() {
    RECT clientRectangle{};
    if (GetClientRect(window_, &clientRectangle) == FALSE) {
        throw std::runtime_error("Failed to query the window dimensions.");
    }

    const UINT newWidth = static_cast<UINT>(clientRectangle.right - clientRectangle.left);
    const UINT newHeight = static_cast<UINT>(clientRectangle.bottom - clientRectangle.top);
    if (newWidth == 0 || newHeight == 0 || (newWidth == width_ && newHeight == height_)) {
        return;
    }

    context_->OMSetRenderTargets(0, nullptr, nullptr);
    renderTarget_.Reset();
    ThrowIfFailed(
        swapChain_->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0),
        "Failed to resize the swap chain.");
    width_ = newWidth;
    height_ = newHeight;
    CreateRenderTarget();
}

} // namespace trifix
