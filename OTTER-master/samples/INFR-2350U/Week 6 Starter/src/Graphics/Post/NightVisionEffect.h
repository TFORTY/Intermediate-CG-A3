#pragma once

#include "Graphics/Post/PostEffect.h"

class NightVisionEffect : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;

	void ApplyEffect(PostEffect* buffer) override;

	float GetLuminosity();

	void SetLuminosity(float luminosity);

private:
	float _luminosity = 1.0f;
};