#pragma once
#include <vector>

struct AnimationClip
{
	std::vector<int> frames;
	float frameDuration = 0.1f;
	bool loop = true;
};
