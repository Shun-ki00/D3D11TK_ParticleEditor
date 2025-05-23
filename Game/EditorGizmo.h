#pragma once

class CommonResources;


class EditorGizmo
{
public:

	// TK�񋟂̃��b�p�[����z��ɕϊ�
	void MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat);
	// TK�񋟂̃��b�p�[����z��ɕϊ�
	void FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat);

public:

	// �~�̕`��
	void DrawCircle(const DirectX::SimpleMath::Vector3& center, const float& radius, const DirectX::FXMVECTOR& color, const int& split);
	// ���̕`��
	void DrawLine(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& vector, const DirectX::FXMVECTOR& color);

	// �O���b�h��`��
	void DrawGrid();

	// �M�Y����`��
	DirectX::SimpleMath::Matrix DrawManipulate(const DirectX::SimpleMath::Matrix& worldMatrix , ImGuizmo::OPERATION operation , ImGuizmo::MODE mode);


	// ����������
	void Initialize();

	EditorGizmo();
	~EditorGizmo() = default;

private:

	// ���L���\�[�X
	CommonResources* m_commonResources;
	// �f�o�C�X
	ID3D11Device* m_device;
	// �f�o�C�X�R���e�L�X�g
	ID3D11DeviceContext* m_context;
	// �R�����X�e�[�g
	DirectX::CommonStates* m_commonStates;


	// �X�v���C�g�o�b�`
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	// �X�v���C�g�t�H���g
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	// �x�[�V�b�N�G�t�F�N�g
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	// �v���~�e�B�u�o�b�`
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;
	// �G�t�F�N�g�t�@�N�g���[
	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;
	// ���X�^���C�U�[�X�e�[�g
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterrizerState;
	// ���̓��C�A�E�g
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};