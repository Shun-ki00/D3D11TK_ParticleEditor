#pragma once
#include "Game/Parameters/ParameterBuffers.h"
#include "Game/Particle/Particle.h"
#include "Game/EditorGizmo.h"

class Particle;
class Parameters;
class EditorGizmo;

class ParticleEmitter
{
private:

	enum class Shape
	{
		CONE,
		SPHERE,
	};

public:

	// �C���v�b�g�f�[�^���擾����
	std::vector<DirectX::VertexPositionColorTexture> GetInputDatas() const { return m_inputDatas; }
	// �s�N�Z���V�F�[�_���擾����
	ID3D11PixelShader* GetPixelShader() const { return m_pixelShader; }

	// �e�N�X�`�����擾����
	ID3D11ShaderResourceView* GetTexture() { return m_texture; }

	// ���[���h�s����擾����
	DirectX::SimpleMath::Matrix GetWorldMatrix() const { return m_worldMatrix; }
	// ���[���h�s���ݒ肷��
	void SetWorldMatrix(const DirectX::SimpleMath::Matrix& world) { m_worldMatrix = world; }

	// �p�����[�^�f�[�^��ݒ�
	void SetParticleParameters(const ParticleParameters& parameters) { m_particleParameters = parameters; }


public:

	// �R���X�g���N�^
	ParticleEmitter(const ParticleParameters& parametersData);
	// �f�X�g���N�^
	~ParticleEmitter() = default;

	// ����������
	void Initialize();

	// �X�V����
	void Update(const float& elapsedTime);

	void Emit();

	void Play();

	void Stop();

	void DebugDraw();

	void DebugWidnow();


private:

	// �����_������
	float RandomFloat(float min, float max);

	// �R�[����̃����_������
	void GenerateConeEmission(
		float coneAngleDeg,
		float coneRadius,
		float coneHeight,
		bool emitFromShell,
		const DirectX::SimpleMath::Vector3& coneOrigin,
		const DirectX::SimpleMath::Vector3& coneDirection, // �K�����K�����ēn��
		DirectX::SimpleMath::Vector3& outPosition,
		DirectX::SimpleMath::Vector3& outVelocity);

	// �X�t�B�A�^�̃����_��
	void GenerateSphereEmission(
		float sphereRadius,
		bool emitFromShell,
		const DirectX::SimpleMath::Vector3& center,
		float randomDirectionStrength,
		DirectX::SimpleMath::Vector3& outPosition,
		DirectX::SimpleMath::Vector3& outVelocity);

private:

	// �p�����[�^
	Parameters* m_parameters;

	// �p�[�e�B�N�����q
	std::vector<std::unique_ptr<Particle>> m_particles;
	std::vector<Particle*> m_activeParticles;
	// �C���v�b�g�f�[�^
	std::vector<DirectX::VertexPositionColorTexture> m_inputDatas;
	// �e�N�X�`��
	ID3D11ShaderResourceView* m_texture;
	// �s�N�Z���V�F�[�_�[
	ID3D11PixelShader* m_pixelShader;

	// �p�[�e�B�N���p�����[�^
	ParticleParameters m_particleParameters;

	Shape m_shape;

	float m_elapsedTime;
	float m_duration;

	// �f�o�b�O�`��
	std::unique_ptr<EditorGizmo> m_editorGizmo;

	// ���[���h�s��
	DirectX::SimpleMath::Matrix m_worldMatrix;

};