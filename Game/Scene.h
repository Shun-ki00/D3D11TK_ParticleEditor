#pragma once
#include <future>
#include "Framework/DebugCamera.h"
#include "Framework/Microsoft/RenderTexture.h"
#include "Game/ConstantBuffer.h"
#include "Game/Particle/ParticleEmitter.h"

class CommonResources;
class DebugCamera;
class ParticleEmitter;

class Scene
{
private:

	struct ParticleConstBuffer
	{
		DirectX::SimpleMath::Matrix worldMatrix;
		DirectX::SimpleMath::Matrix viewMatrix;
		DirectX::SimpleMath::Matrix projectionMatrix;
		DirectX::SimpleMath::Matrix billboardMatrix;
		DirectX::SimpleMath::Vector4 time;
	};


public:

	using json = nlohmann::json;

	// �R���X�g���N�^
	Scene();
	// �f�X�g���N�^
	~Scene() = default;

public:

	// ����������
	void Initialize();
	// �X�V����
	void Update(const float& elapsedTime);
	// �`�揈��
	void Render();
	// �I������
	void Finalize();

private:

	// TK�񋟂̃��b�p�[����z��ɕϊ�
	void MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat);
	// TK�񋟂̃��b�p�[����z��ɕϊ�
	void FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix,const float* mat);

	// �V�F�[�_�[�ƃo�b�t�@�̍쐬
	void CreateShaderAndBuffer();
	// �p�[�e�B�N���`�揈��
	void ParticleRender(const DirectX::SimpleMath::Matrix& worldMatrix, 
		const DirectX::SimpleMath::Matrix& viewMatrix, const DirectX::SimpleMath::Matrix& projectionMatrix);

	// �p�[�e�B�N���f�[�^�̕ҏW
	void ParticleDataEditor();

private:

	// ���L���\�[�X
	CommonResources* m_commonResources;

	// �f�o�C�X
	ID3D11Device1* m_device;
	// �R���e�L�X�g
	ID3D11DeviceContext1* m_context;
	// �R�����X�e�[�g
	DirectX::CommonStates* m_commonStates;
	
	// ���f�����W
	DirectX::SimpleMath::Vector3 m_position;
	// ���f����]
	DirectX::SimpleMath::Quaternion m_rotation;
	// ���f���X�P�[��
	DirectX::SimpleMath::Vector3 m_scale;

	// ���f�����[���h�s��
	DirectX::SimpleMath::Matrix m_world;

	// ���_�s��
	DirectX::SimpleMath::Matrix m_gridMatrix;
	float m_arrayGridMatrix[16];

	// ���C���J����
	float m_cameraDistance;
	DirectX::SimpleMath::Matrix m_mainViewMatrix;

	// �M�Y���̃��[�h
	ImGuizmo::OPERATION m_operation;
	ImGuizmo::MODE m_mode;

	// �I�t�X�N���[���p�����_�[�e�N�X�`��
	std::unique_ptr<DX::RenderTexture> m_main;
	// �I�t�X�N���[���p�����_�[�e�N�X�`��
	std::unique_ptr<DX::RenderTexture> m_sub;


	// �Œ�r���[�s��
	DirectX::SimpleMath::Matrix m_fixedViewMatrix;
	


	// === �p�[�e�B�N�� ===

	// �p�[�e�B�N��������
	std::unique_ptr<ParticleEmitter> m_particleEmitter;
	// �C���v�b�g���C�A�E�g
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_particleiInputLayout;
	// �萔�o�b�t�@
	std::unique_ptr<ConstantBuffer<ParticleConstBuffer>> m_particleConstBuffer;
	// ���_�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_particleVertexShader;
	// �W�I���g���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_particleGeometryShader;
	// �s�N�Z�����V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_particlePixelShader;
	// ���_�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_particleVertexBuffer;
	// �r���{�[�h
	DirectX::SimpleMath::Matrix m_billboardMatrix;


	// �p�����[�^�f�[�^
	ParticleParameters m_parametersData;

};