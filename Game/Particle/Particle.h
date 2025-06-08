#pragma once

class Particle
{

public:

    DirectX::SimpleMath::Vector3 GetPosition() const { return m_position; }

    float GetScale() const { return m_currentSize; }

    DirectX::SimpleMath::Vector4 GetColor() const { return m_currentColor; }

    bool GetIsActive() const { return m_isActive; }

	Particle();
	~Particle() = default;


    void Initialize(
        const DirectX::SimpleMath::Vector3& position,
        const DirectX::SimpleMath::Vector3& velocity,
        const DirectX::SimpleMath::Vector3& acceleration,
        float lifetime,
        float startSize,
        float endSize,
        const DirectX::SimpleMath::Vector4& startColor,
        const DirectX::SimpleMath::Vector4& endColor
    );

    void Update(const float& elapsedTime);

private:

    bool m_isActive;

    DirectX::VertexPositionColorTexture m_inputData;

    // ��{����
    DirectX::SimpleMath::Vector3 m_position;
    DirectX::SimpleMath::Vector3 m_velocity;
    DirectX::SimpleMath::Vector3 m_acceleration;

    // ���C�t�^�C��
    float m_totalLifetime; // �ő����
    float m_currentLifetime; // �c�����

    // �T�C�Y���
    float m_startSize;
    float m_endSize;
    float m_currentSize;

    // �J���[���
    DirectX::SimpleMath::Vector4 m_startColor;
    DirectX::SimpleMath::Vector4 m_endColor;
    DirectX::SimpleMath::Vector4 m_currentColor;
};