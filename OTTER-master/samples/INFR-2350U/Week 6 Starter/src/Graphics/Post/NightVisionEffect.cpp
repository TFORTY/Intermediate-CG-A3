#include "NightVisionEffect.h"

void NightVisionEffect::Init(unsigned width, unsigned height)
{
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	_buffers[index]->Init(width, height);

	index = int(_shaders.size());
	_shaders.push_back(Shader::Create());
	_shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index]->LoadShaderPartFromFile("shaders/Post/night_vision_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index]->Link();
}

void NightVisionEffect::ApplyEffect(PostEffect* buffer)
{
	BindShader(0);
	_shaders[0]->SetUniform("u_Luminosity", _luminosity);
	buffer->BindColorAsTexture(0, 0, 0);

	_buffers[0]->RenderToFSQ();
	buffer->UnbindTexture(0);
	UnbindShader();
}

float NightVisionEffect::GetLuminosity()
{
	return _luminosity;
}

void NightVisionEffect::SetLuminosity(float luminosity)
{
	_luminosity = luminosity;
}