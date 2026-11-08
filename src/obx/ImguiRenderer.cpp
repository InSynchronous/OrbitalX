#include "ImGuiRenderer.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

bgfx::ShaderHandle loadShader(const char *path) {
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Failed to open ImGui shader: " << path << '\n';
        return BGFX_INVALID_HANDLE;
    }

    std::vector<char> data{std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>()};

    const bgfx::Memory *memory =
        bgfx::copy(data.data(), static_cast<uint32_t>(data.size()));

    return bgfx::createShader(memory);
}

void uploadRect(bgfx::TextureHandle handle, ImTextureData *texture, int x,
                int y, int w, int h) {
    if (w <= 0 || h <= 0)
        return;

    std::vector<uint8_t> pixels(static_cast<size_t>(w) * h * 4);

    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            const uint8_t *src = static_cast<const uint8_t *>(
                texture->GetPixelsAt(x + col, y + row));

            const size_t dst = (static_cast<size_t>(row) * w + col) * 4;

            if (texture->Format == ImTextureFormat_Alpha8) {
                pixels[dst + 0] = 255;
                pixels[dst + 1] = 255;
                pixels[dst + 2] = 255;
                pixels[dst + 3] = src[0];
            } else if (texture->Format == ImTextureFormat_RGBA32) {
                std::memcpy(&pixels[dst], src, 4);
            }
        }
    }
    bgfx::updateTexture2D(
        handle, 0, 0, static_cast<uint16_t>(x), static_cast<uint16_t>(y),
        static_cast<uint16_t>(w), static_cast<uint16_t>(h),
        bgfx::copy(pixels.data(), static_cast<uint32_t>(pixels.size())));
}

void updateTexture(ImTextureData *texture) {
    switch (texture->Status) {
    case ImTextureStatus_WantCreate: {
        if (texture->Format != ImTextureFormat_RGBA32) {
            std::cerr << "Unsupported ImGui texture format\n";
            return;
        }

        bgfx::TextureHandle handle =
            bgfx::createTexture2D(static_cast<uint16_t>(texture->Width),
                                  static_cast<uint16_t>(texture->Height), false,
                                  1, bgfx::TextureFormat::RGBA8,
                                  BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

        if (!bgfx::isValid(handle)) {
            std::cerr << "Failed to create ImGui texture\n";
            return;
        }

        uploadRect(handle, texture, 0, 0, texture->Width, texture->Height);

        texture->SetTexID(handle.idx);
        texture->SetStatus(ImTextureStatus_OK);
        break;
    }

    case ImTextureStatus_WantUpdates: {
        bgfx::TextureHandle handle;
        handle.idx = static_cast<uint16_t>(texture->GetTexID());

        const auto &rect = texture->UpdateRect;

        uploadRect(handle, texture, rect.x, rect.y, rect.w, rect.h);

        for (const ImTextureRect &update : texture->Updates) {
            uploadRect(handle, texture, update.x, update.y, update.w, update.h);
        }

        texture->SetStatus(ImTextureStatus_OK);
        break;
    }

    case ImTextureStatus_WantDestroy: {
        if (texture->UnusedFrames <= 0)
            break;

        bgfx::TextureHandle handle;
        handle.idx = static_cast<uint16_t>(texture->GetTexID());

        if (bgfx::isValid(handle))
            bgfx::destroy(handle);

        texture->SetTexID(ImTextureID_Invalid);
        texture->SetStatus(ImTextureStatus_Destroyed);
        break;
    }

    default:
        break;
    }
}

void ImGuiRenderer::initialize() {
    layout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    const char *vsPath = nullptr;
    const char *fsPath = nullptr;

    switch (bgfx::getRendererType()) {
    case bgfx::RendererType::Metal:
        vsPath = "assets/shaders/metal/vs_imgui.bin";
        fsPath = "assets/shaders/metal/fs_imgui.bin";
        break;
    default:
        vsPath = "assets/shaders/spirv/vs_imgui.bin";
        fsPath = "assets/shaders/spirv/fs_imgui.bin";
        break;
    }

    bgfx::ShaderHandle vertexShader = loadShader(vsPath);

    bgfx::ShaderHandle fragmentShader = loadShader(fsPath);

    if (!bgfx::isValid(vertexShader) || !bgfx::isValid(fragmentShader)) {
        std::cerr << "Failed to load ImGui shaders\n";

        if (bgfx::isValid(vertexShader))
            bgfx::destroy(vertexShader);

        if (bgfx::isValid(fragmentShader))
            bgfx::destroy(fragmentShader);

        return;
    }

    program = bgfx::createProgram(vertexShader, fragmentShader, true);

    sampler = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

    ImGuiIO &io = ImGui::GetIO();

    io.BackendRendererName = "OrbitalX bgfx renderer";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
}

void ImGuiRenderer::render(ImDrawData *drawData) {
    if (!drawData || !bgfx::isValid(program))
        return;

    const int fbWidth = static_cast<int>(drawData->DisplaySize.x *
                                         drawData->FramebufferScale.x);

    const int fbHeight = static_cast<int>(drawData->DisplaySize.y *
                                          drawData->FramebufferScale.y);

    if (fbWidth <= 0 || fbHeight <= 0)
        return;

    if (drawData->Textures) {
        for (ImTextureData *texture : *drawData->Textures) {
            if (texture->Status != ImTextureStatus_OK)
                updateTexture(texture);
        }
    }

    const float left = drawData->DisplayPos.x;
    const float right = left + drawData->DisplaySize.x;
    const float top = drawData->DisplayPos.y;
    const float bottom = top + drawData->DisplaySize.y;

    float projection[16];

    bx::mtxOrtho(projection, left, right, bottom, top, 0.0f, 1000.0f, 0.0f,
                 bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
    bgfx::setViewRect(viewId, 0, 0, static_cast<uint16_t>(fbWidth),
                      static_cast<uint16_t>(fbHeight));

    bgfx::setViewTransform(viewId, nullptr, projection);

    const uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                           BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA;
    const ImVec2 clipOffset = drawData->DisplayPos;
    const ImVec2 clipScale = drawData->FramebufferScale;

    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList *cmdList = drawData->CmdLists[n];

        const uint32_t vertexCount =
            static_cast<uint32_t>(cmdList->VtxBuffer.Size);

        const uint32_t indexCount =
            static_cast<uint32_t>(cmdList->IdxBuffer.Size);

        if (vertexCount == 0 || indexCount == 0)
            continue;

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        if (bgfx::getAvailTransientVertexBuffer(vertexCount, layout) <
                vertexCount ||
            bgfx::getAvailTransientIndexBuffer(indexCount) < indexCount) {
            continue;
        }

        bgfx::allocTransientVertexBuffer(&tvb, vertexCount, layout);

        bgfx::allocTransientIndexBuffer(&tib, indexCount);

        std::memcpy(tvb.data, cmdList->VtxBuffer.Data,
                    vertexCount * sizeof(ImDrawVert));

        std::memcpy(tib.data, cmdList->IdxBuffer.Data,
                    indexCount * sizeof(ImDrawIdx));

        for (const ImDrawCmd &cmd : cmdList->CmdBuffer) {
            if (cmd.UserCallback) {
                if (cmd.UserCallback == ImDrawCallback_ResetRenderState)
                    continue;

                cmd.UserCallback(cmdList, &cmd);
                continue;
            }

            ImVec2 clipMin((cmd.ClipRect.x - clipOffset.x) * clipScale.x,
                           (cmd.ClipRect.y - clipOffset.y) * clipScale.y);

            ImVec2 clipMax((cmd.ClipRect.z - clipOffset.x) * clipScale.x,
                           (cmd.ClipRect.w - clipOffset.y) * clipScale.y);

            clipMin.x = std::max(clipMin.x, 0.0f);
            clipMin.y = std::max(clipMin.y, 0.0f);
            clipMax.x = std::min(clipMax.x, static_cast<float>(fbWidth));
            clipMax.y = std::min(clipMax.y, static_cast<float>(fbHeight));

            if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                continue;

            const uint16_t scissorX = static_cast<uint16_t>(clipMin.x);

            const uint16_t scissorY = static_cast<uint16_t>(clipMin.y);

            const uint16_t scissorWidth =
                static_cast<uint16_t>(clipMax.x - clipMin.x);

            const uint16_t scissorHeight =
                static_cast<uint16_t>(clipMax.y - clipMin.y);

            bgfx::setScissor(scissorX, scissorY, scissorWidth, scissorHeight);

            ImTextureID textureID = cmd.GetTexID();

            bgfx::TextureHandle texture;
            texture.idx = static_cast<uint16_t>(textureID);

            if (!bgfx::isValid(texture)) {
                std::cerr << "Invalid ImGui texture: " << textureID << '\n';
                continue;
            }

            bgfx::setTexture(0, sampler, texture);

            bgfx::setVertexBuffer(0, &tvb, cmd.VtxOffset, vertexCount);

            bgfx::setIndexBuffer(&tib, cmd.IdxOffset, cmd.ElemCount);

            bgfx::setState(state);

            bgfx::submit(viewId, program);
        }
    }

    if (drawData->CmdListsCount == 0)
        bgfx::touch(viewId);
}

void ImGuiRenderer::shutdown() {
    for (ImTextureData *texture : ImGui::GetPlatformIO().Textures) {
        if (texture->Status == ImTextureStatus_Destroyed)
            continue;

        bgfx::TextureHandle handle;
        handle.idx = static_cast<uint16_t>(texture->GetTexID());

        if (bgfx::isValid(handle))
            bgfx::destroy(handle);

        texture->SetTexID(ImTextureID_Invalid);
        texture->SetStatus(ImTextureStatus_Destroyed);
    }

    if (bgfx::isValid(sampler))
        bgfx::destroy(sampler);

    if (bgfx::isValid(program))
        bgfx::destroy(program);

    ImGuiIO &io = ImGui::GetIO();

    io.BackendRendererName = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;

    sampler = BGFX_INVALID_HANDLE;
    program = BGFX_INVALID_HANDLE;
}
