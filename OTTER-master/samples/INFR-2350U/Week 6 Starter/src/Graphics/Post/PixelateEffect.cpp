#include "PixelateEffect.h"

void PixelateEffect::Init(unsigned width, unsigned height)
{
	//Load the buffers
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	//_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

	//Load the shaders
	index = int(_shaders.size());
	_shaders.push_back(Shader::Create());
	_shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index]->LoadShaderPartFromFile("shaders/Post/pixelate_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index]->Link();

	_windowSize = glm::vec2(float(width), float(height));
}

void PixelateEffect::ApplyEffect(PostEffect* buffer)
{
	BindShader(0);
	_shaders[0]->SetUniform("u_WindowSize", _windowSize);
	_shaders[0]->SetUniform("u_PixelSize", _pixelSize);

	buffer->BindColorAsTexture(0, 0, 0);

	_buffers[0]->RenderToFSQ();

	buffer->UnbindTexture(0);

	UnbindShader();
}

glm::vec2 PixelateEffect::GetWindowSize() const
{
	return _windowSize;
}

float PixelateEffect::GetPixelSize() const
{
	return _pixelSize;
}

void PixelateEffect::SetPixelSize(float size)
{
	_pixelSize = size;
}

void PixelateEffect::SetWindowSize(glm::vec2 size)
{
	_windowSize = size;
}