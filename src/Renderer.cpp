#include "TriFix/Renderer.h"

#include <dxgi.h>

#include <iterator>
#include <stdexcept>

namespace {

void ThrowIfFailed(HRESULT result, const char* message) {
    if (FAILED(result)) {
        throw std::runtime_error(message);
    }
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
}

void Renderer::Render() {
    ResizeIfNeeded();

    constexpr float clearColor[] = {0.10F, 0.10F, 0.10F, 1.0F};
    context_->OMSetRenderTargets(1, renderTarget_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(renderTarget_.Get(), clearColor);
    ThrowIfFailed(swapChain_->Present(1, 0), "Failed to present a rendered frame.");
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
