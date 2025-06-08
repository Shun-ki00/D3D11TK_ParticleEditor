#include "pch.h"
#include "Game/Particle/Particle.h"

Particle::Particle()
{
    m_isActive = false;
}

void Particle::Initialize(
    const DirectX::SimpleMath::Vector3& position,
    const DirectX::SimpleMath::Vector3& velocity,
    const DirectX::SimpleMath::Vector3& acceleration,
    float lifetime,
    float startSize,
    float endSize,
    const DirectX::SimpleMath::Vector4& startColor,
    const DirectX::SimpleMath::Vector4& endColor
)
{
    m_position = position;
    m_velocity = velocity;
    m_acceleration = acceleration;
    m_totalLifetime = lifetime;
    m_currentLifetime = m_totalLifetime;
    m_startSize = startSize;
    m_endSize = endSize;
    m_startColor = startColor;
    m_endColor = endColor;

    m_isActive = true;
}

void Particle::Update(const float& elapsedTime)
{
    if (!m_isActive) return;

    // ���Ԃ����炷
    m_currentLifetime -= elapsedTime;
    if (m_currentLifetime <= 0.0f) {
        m_currentLifetime = 0.0f;

        m_isActive = false;
        return;
    }

    // �ʒu�X�V
    m_velocity += m_acceleration * elapsedTime;
    m_position += m_velocity * elapsedTime;

    // �c�莞�Ԃ̊���
    float lifeRatio = (m_totalLifetime > 0.0f) ? (1.0f - (m_currentLifetime / m_totalLifetime)) : 1.0f;
    lifeRatio = std::clamp(lifeRatio, 0.0f, 1.0f);

    // �T�C�Y���
    m_currentSize = m_startSize + (m_endSize - m_startSize) * lifeRatio;

    // �J���[���
    m_currentColor = m_startColor + (m_endColor - m_startColor) * lifeRatio;
}