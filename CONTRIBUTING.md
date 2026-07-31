# Contributing to TriFix

## Project Philosophy

- Prioritise readability over cleverness.
- Keep the architecture modular.
- Build the project incrementally through small, testable milestones.
- Ensure every commit compiles successfully.

## Architecture

Maintain a clear separation of responsibilities:

- `Application` manages the application lifecycle.
- `Window` manages the Win32 window and message handling.
- `Renderer` manages Direct3D resources and rendering.
- Specialised renderers, such as `GridRenderer`, `WarpRenderer`, and
  `TextureRenderer`, should remain independent components rather than expanding
  the core renderer.

## Coding Style

- Use modern C++20.
- Prefer RAII.
- Prefer smart pointers and `Microsoft::WRL::ComPtr` for resource ownership.
- Minimise global variables.
- Keep functions concise and focused.
- Comment why something is implemented, not merely what it does.

## Git Workflow

- Keep each commit limited to one logical feature or fix.
- Avoid unrelated refactoring.
- Write clear, descriptive commit messages.
- Open a pull request for review before merging.

## Project Goals

The long-term objective of TriFix is to become a GPU-based triple-monitor
reprojection utility capable of:

- Perspective-correct image reprojection.
- Physical monitor geometry calibration.
- Low-latency rendering.
- Desktop and game frame capture.
- A modular rendering architecture suitable for future expansion.

## Out of Scope (for Now)

- ImGui
- JSON configuration
- Plug-in systems
- Third-party rendering libraries
- Premature optimisation
