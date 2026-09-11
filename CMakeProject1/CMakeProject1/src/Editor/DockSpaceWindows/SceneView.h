#pragma once
class SceneView
{
public:
	SceneView();

	void Draw();

	unsigned int GetFramebuffer() const;
	unsigned int GetTexture() const;

private:
	unsigned int m_Framebuffer = 0;
	unsigned int m_Texture = 0;
	unsigned int m_DepthBuffer = 0;
	int m_Width = 800;
	int m_Height = 600;
};