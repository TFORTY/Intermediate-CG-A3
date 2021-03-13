#include "FilmGrainEffect.h"
#include "GLFW/glfw3.h"

void FilmGrainEffect::Init(unsigned width, unsigned height)
{
	//Load the buffers
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	//_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

	//Load the shaders
	int index2 = int(_shaders.size());
	_shaders.push_back(Shader::Create());
	_shaders[index2]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index2]->LoadShaderPartFromFile("shaders/Post/film_grain_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index2]->Link();

	_windowSize = glm::vec2(float(width), float(height));
}

void FilmGrainEffect::ApplyEffect(PostEffect* buffer)
{
	_time = glfwGetTime();

	BindShader(0);
	_shaders[0]->SetUniform("u_WindowSize", _windowSize);
	_shaders[0]->SetUniform("u_Strength", _strength);
	_shaders[0]->SetUniform("u_Time", _time);

	buffer->BindColorAsTexture(0, 0, 0);

	_buffers[0]->RenderToFSQ();

	buffer->UnbindTexture(0);

	UnbindShader();
}

glm::vec2 FilmGrainEffect::GetWindowSize() const
{
	return _windowSize;
}

float FilmGrainEffect::GetStrength() const
{
	return _strength;
}

float FilmGrainEffect::GetTime() const
{
	return _time;
}

void FilmGrainEffect::SetStrength(float strength)
{
	_strength = strength;
}

void FilmGrainEffect::SetWindowSize(glm::vec2 size)
{
	_windowSize = size;
}