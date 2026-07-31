#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace trifix {

class Renderer final {
public:
    explicit Renderer(HWND window);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Render();

private:
    void CreateRenderTarget();
    void ResizeIfNeeded();

    HWND window_ = nullptr;
    UINT width_ = 0;
    UINT height_ = 0;
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget_;
};

} // namespace trifix
