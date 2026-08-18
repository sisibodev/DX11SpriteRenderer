#include "render/AnimatedSprite.h"

void AnimatedSprite::SetClip(const AnimationClip* clip)
{
	if (m_clip == clip) return;
	m_clip = clip;
	m_frameIndex = 0;
	m_elapsed = 0;
}

void AnimatedSprite::Update(float dt)
{
	if (m_clip == nullptr || m_clip->frames.empty()) return;

	m_elapsed += dt;

	while (m_elapsed >= m_clip->frameDuration)
	{
		m_elapsed -= m_clip->frameDuration;
		++m_frameIndex;

		if (m_frameIndex >= static_cast<int>(m_clip->frames.size()))
		{
			if (m_clip->loop)
			{
				m_frameIndex = 0;
			}
			else
			{
				m_frameIndex = static_cast<int>(m_clip->frames.size()) - 1;
				m_elapsed = 0.0f;
				break;
			}
		}
	}
}

int AnimatedSprite::GetCurrentTile() const
{
	if (m_clip == nullptr || m_clip->frames.empty()) return 0;

	return m_clip->frames[m_frameIndex];
}

