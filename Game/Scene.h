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
public:

	// �ő嗱�q�̐�
	static const int MAX_PARTICLE_VERTEX_COUNT;

	// Scene�E�B���h�E�̍��W�ƃT�C�Y
	static const ImVec2 MAIN_WINDOW_POSITION;
	static const ImVec2 MAIN_WINDOW_SIZE;

	// Scene�E�B���h�E�̍��W�ƃT�C�Y
	static const ImVec2 SCENE_WINDOW_POSITION;
	static const ImVec2 SCENE_WINDOW_SIZE;

	// Particle Settings�E�B���h�E�̍��W�ƃT�C�Y
	static const ImVec2 PARTICLE_SETTINGS_WINDOW_POSITION;
	static const ImVec2 PARTICLE_SETTINGS_WINDOW_SIZE;


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

	// �V�F�[�_�[�ƃo�b�t�@�̍쐬
	void CreateShaderAndBuffer();
	// �p�[�e�B�N���`�揈��
	void ParticleRender(const DirectX::SimpleMath::Matrix& worldMatrix, 
		const DirectX::SimpleMath::Matrix& viewMatrix, const DirectX::SimpleMath::Matrix& projectionMatrix);

	// �p�[�e�B�N���f�[�^�̕ҏW
	void ParticleDataEditor();

	// ���C���V�[����`��
	void DrawMainScene();


	// ==== ImGui�E�B���h�E�쐬 ===

	// ���C�A�E�g��ݒ�
	void SetupMainLayout();
	// �V�[���E�B���h�E

	// �l�ݒ�E�B���h�E

	// ���j���[�o�[
	void DrawMenuBar();

	// �{�^���E�B���h�E




private:

	// ==== �t���[�����[�N ====

	// ���L���\�[�X
	CommonResources* m_commonResources;

	// �f�o�C�X
	ID3D11Device1* m_device;
	// �R���e�L�X�g
	ID3D11DeviceContext1* m_context;
	// �R�����X�e�[�g
	DirectX::CommonStates* m_commonStates;


	// ==== �A�N�e�B�u�ݒ� ====

	// �O���b�h�̃A�N�e�B�u�ݒ�
	bool m_isGridActive;
	// ���̃A�N�e�B�u�ݒ�
	bool m_isAxisActive;
	

	// ���_�s��
	DirectX::SimpleMath::Matrix m_gridMatrix;
	float m_arrayGridMatrix[16];

	// �f�o�b�O�J����
	std::unique_ptr<DebugCamera> m_debugCamera;
	// �E�B���h�E�ɐG��Ă��邩�ǂ���
	bool m_sceneAllowCameraInput;

	// �I�t�X�N���[���p�����_�[�e�N�X�`��
	std::unique_ptr<DX::RenderTexture> m_main;
	// �I�t�X�N���[���p�����_�[�e�N�X�`��
	std::unique_ptr<DX::RenderTexture> m_sub;


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

	// �e�N�X�`���R���e�i
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;

	// �p�����[�^�f�[�^
	ParticleParameters m_parametersData;

};