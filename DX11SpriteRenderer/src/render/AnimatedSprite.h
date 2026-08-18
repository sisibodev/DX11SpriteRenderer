#pragma once
#include "render/AnimationClip.h"

class AnimatedSprite
{
public:
	void SetClip(const AnimationClip* clip);
	void Update(float dt);
	int GetCurrentTile() const;

private:
	const AnimationClip* m_clip = nullptr;
	float m_elapsed = 0.0f;
	int m_frameIndex = 0;
};
