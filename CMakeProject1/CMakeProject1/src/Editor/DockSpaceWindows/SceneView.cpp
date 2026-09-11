#include "SceneView.h"
#include "imgui.h"
#include <algorithm>
#include <glad/glad.h>
SceneView::SceneView()
{
     // Framebuffer
     glGenFramebuffers(1, &m_Framebuffer);
     glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

     // ============================================================
     // Color Texture
     // ============================================================

     glGenTextures(1, &m_Texture);

     glBindTexture(GL_TEXTURE_2D, m_Texture);

     glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGB,
          m_Width,
          m_Height,
          0,
          GL_RGB,
          GL_UNSIGNED_BYTE,
          nullptr
     );

     glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_MIN_FILTER,
          GL_LINEAR
     );

     glTexParameteri(
          GL_TEXTURE_2D,
          GL_TEXTURE_MAG_FILTER,
          GL_LINEAR
     );

     glFramebufferTexture2D(
          GL_FRAMEBUFFER,
          GL_COLOR_ATTACHMENT0,
          GL_TEXTURE_2D,
          m_Texture,
          0
     );

     // ============================================================
     // Depth Renderbuffer
     // ============================================================

     glGenRenderbuffers(1, &m_DepthBuffer);

     glBindRenderbuffer(
          GL_RENDERBUFFER,
          m_DepthBuffer
     );

     glRenderbufferStorage(
          GL_RENDERBUFFER,
          GL_DEPTH24_STENCIL8,
          m_Width,
          m_Height
     );

     glFramebufferRenderbuffer(
          GL_FRAMEBUFFER,
          GL_DEPTH_STENCIL_ATTACHMENT,
          GL_RENDERBUFFER,
          m_DepthBuffer
     );



     glBindRenderbuffer(GL_RENDERBUFFER, 0);
     glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneView::Draw() {
     ImGui::Begin("SceneView");

     ImVec2 availSize = ImGui::GetContentRegionAvail();  // 当前可用区域大小

     // 获取纹理实际尺寸（假设已存储为 m_Width, m_Height）
     // 若未存储，可通过 glGetTexLevelParameteriv 实时获取
     GLint texW = m_Width, texH = m_Height;  // 请确保这两个成员已正确初始化

     // 计算 cover 模式的缩放因子
     float scale = std::max(availSize.x / texW, availSize.y / texH);
     float scaledW = texW * scale;
     float scaledH = texH * scale;

     // 计算居中偏移（单位：像素，在缩放后的空间中）
     float offsetX = (scaledW - availSize.x) * 0.5f;
     float offsetY = (scaledH - availSize.y) * 0.5f;

     // 转换为 UV 坐标（相对于原纹理 [0,1]）
     ImVec2 uv0(offsetX / scaledW, (offsetY + availSize.y) / scaledH);
     ImVec2 uv1((offsetX + availSize.x) / scaledW, offsetY / scaledH);
     // 获取当前窗口的 DrawList
     ImDrawList* drawList = ImGui::GetWindowDrawList();
     ImVec2 posMin = ImGui::GetCursorScreenPos();          // 左上角
     ImVec2 posMax(posMin.x + availSize.x, posMin.y + availSize.y);

     // 绘制裁剪后的纹理
     ImGui::Image((ImTextureID)(intptr_t)m_Texture, availSize, uv0, uv1);

     ImGui::End();


}

unsigned int SceneView::GetFramebuffer() const {
     return m_Framebuffer;

}
unsigned int SceneView::GetTexture() const {
     return m_Texture;

}