#include "pch.h"
#include "Framework/RandomUtillities.h"
#include <random>



float RandomUtillities::RandomFloat(float min, float max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}




void RandomUtillities::GenerateConeEmissio
(
	float coneAngleDeg,          // �R�[���̊p�x
	float radius,            // �R�[���̔��a
	float coneHeight,            // �R�[���̍����i�X�s�[�h�j
	bool emitFromShell,
	const DirectX::SimpleMath::Vector3& coneOrigin, // �R�[���̌��_
	DirectX::SimpleMath::Vector3& outPosition,
	DirectX::SimpleMath::Vector3& outVelocity)
{

    using namespace DirectX::SimpleMath;

    // �~����̃����_���_���擾
    float theta = RandomFloat(0.0f, DirectX::XM_2PI);
    float r = emitFromShell ? radius : RandomFloat(0.0f, radius);
    Vector3 localBase(r * cosf(theta), 0.0f, r * sinf(theta));
    outPosition = coneOrigin + localBase;

    // ���S���_�̊O�����x�N�g��
    Vector3 outward = localBase;
    outward.Normalize();

    // ������x�N�g���� blend ���� coneAngle �������X����
    float coneAngleRad = DirectX::XMConvertToRadians(coneAngleDeg);
    float blendFactor = cosf(coneAngleRad);  // blendFactor: 1.0 = �����, 0.0 = ����
    Vector3 direction = Vector3::Lerp(outward, Vector3::Up, blendFactor);
    direction.Normalize();

    outVelocity = direction * coneHeight;
}



void RandomUtillities::GenerateSphereEmission(
    float sphereRadius,
    bool emitFromShell,
    const DirectX::SimpleMath::Vector3& center,
    float randomDirectionStrength,
    DirectX::SimpleMath::Vector3& outPosition,
    DirectX::SimpleMath::Vector3& outVelocity)
{
    // ���ʏ�̃����_���ȒP�ʃx�N�g��
    float theta = RandomFloat(0.0f, DirectX::XM_2PI);
    float phi = acosf(RandomFloat(-1.0f, 1.0f)); // �ӂ̕��z�ɒ��Ӂi���ʋψ�j

    float x = sinf(phi) * cosf(theta);
    float y = cosf(phi);
    float z = sinf(phi) * sinf(theta);

    DirectX::SimpleMath::Vector3 direction = DirectX::SimpleMath::Vector3(x, y, z);

    float radius = emitFromShell ? sphereRadius : RandomFloat(0.0f, sphereRadius);
    outPosition = center + direction * radius;

    // �O�����ɏ����u���������x�N�g����Ԃ�
    DirectX::SimpleMath::Vector3 randomDir = direction +
        DirectX::SimpleMath::Vector3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1))
        * randomDirectionStrength;
    randomDir.Normalize();

    outVelocity = randomDir;
}