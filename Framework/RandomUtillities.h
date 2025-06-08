#pragma once


class RandomUtillities
{
public:

	// �����_������
	static float RandomFloat(float min, float max);

	// �R�[����̃����_������
	static DirectX::SimpleMath::Vector3 GenerateConeEmissio
	(
		float coneAngleDeg,          // �R�[���̊p�x
		float radius,            // �R�[���̔��a
		float coneHeight,            // �R�[���̍����i�X�s�[�h�j
		bool emitFromShell,
		const DirectX::SimpleMath::Vector3& coneOrigin, // �R�[���̌��_
		DirectX::SimpleMath::Vector3& outPosition,
		DirectX::SimpleMath::Vector3& outVelocity);

	// �X�t�B�A�^�̃����_��
	static DirectX::SimpleMath::Vector3 GenerateSphereEmission(
		float sphereRadius,
		bool emitFromShell,
		const DirectX::SimpleMath::Vector3& center,
		float randomDirectionStrength,
		DirectX::SimpleMath::Vector3& outPosition,
		DirectX::SimpleMath::Vector3& outVelocity);
};