#pragma once
#include <future>
#include "Framework/DebugCamera.h"
#include "Framework/Microsoft/RenderTexture.h"

class CommonResources;
class DebugCamera;

class Scene
{
public:

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

	// Transform�G�f�B�^
	void EditTransform();

private:

	// ���L���\�[�X
	CommonResources* m_commonResources;

	// �f�o�b�O�J����
	std::unique_ptr<DebugCamera> m_camera;

	// �f�o�C�X
	ID3D11Device1* m_device;
	// �R���e�L�X�g
	ID3D11DeviceContext1* m_context;
	
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

	// �M�Y���̃��[�h
	ImGuizmo::OPERATION m_operation;
	ImGuizmo::MODE m_mode;





	// �I�t�X�N���[���p�����_�[�e�N�X�`��
	std::unique_ptr<DX::RenderTexture> m_main;
	// �I�t�X�N���[���p�����_�[�e�N�X�`��
	std::unique_ptr<DX::RenderTexture> m_sub;
};