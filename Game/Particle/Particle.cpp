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

    // 時間を減らす
    m_currentLifetime -= elapsedTime;
    if (m_currentLifetime <= 0.0f) {
        m_currentLifetime = 0.0f;

        m_isActive = false;
        return;
    }

    // 位置更新
    m_velocity += m_acceleration * elapsedTime;
    m_position += m_velocity * elapsedTime;

    // 残り時間の割合
    float lifeRatio = (m_totalLifetime > 0.0f) ? (1.0f - (m_currentLifetime / m_totalLifetime)) : 1.0f;
    lifeRatio = std::clamp(lifeRatio, 0.0f, 1.0f);

    // サイズ補間
    m_currentSize = m_startSize + (m_endSize - m_startSize) * lifeRatio;

    // カラー補間
    m_currentColor = m_startColor + (m_endColor - m_startColor) * lifeRatio;
}