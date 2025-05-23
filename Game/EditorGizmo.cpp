#include "pch.h"
#include "Game/EditorGizmo.h"
#include "Framework/Microsoft/DebugDraw.h"
#include "Framework/CommonResources.h"


EditorGizmo::EditorGizmo()
{
	m_commonResources = CommonResources::GetInstance();
}


void EditorGizmo::Initialize()
{
	m_device = m_commonResources->GetDeviceResources()->GetD3DDevice();
	// �f�o�C�X�R���e�L�X�g���擾����
	m_context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	// �R�����X�e�[�g���擾����
	m_commonStates = m_commonResources->GetCommonStates();

	// �X�v���C�g�o�b�`�𐶐�����
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_context);
	// �x�[�V�b�N�G�t�F�N�g�𐶐�����
	m_basicEffect = std::make_unique<DirectX::BasicEffect>(m_device);
	// �X�v���C�g�t�H���g�𐶐�����
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(m_device, L"resources\\font\\SegoeUI_18.spritefont");
	// �v���~�e�B�u�o�b�`�𐶐�����
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(m_context);
	// ���_�J���[��L���ɂ���
	m_basicEffect->SetVertexColorEnabled(true);
	// �e�N�X�`���𖳌��ɂ���
	m_basicEffect->SetTextureEnabled(false);
	void const* shaderByteCode;
	size_t byteCodeLength;
	// ���_�V�F�[�_�[���擾����
	m_basicEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
	// ���̓��C�A�E�g�𐶐�����
	m_device->CreateInputLayout(
		DirectX::VertexPositionColor::InputElements,
		DirectX::VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		m_inputLayout.ReleaseAndGetAddressOf()
	);
	// ���X�^���C�U�[�f�B�X�N���v�V����
	CD3D11_RASTERIZER_DESC rasterizerStateDesc(
		D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
		D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE
	);
	// ���X�^���C�Y�X�e�[�g�𐶐�����
	m_device->CreateRasterizerState(&rasterizerStateDesc, m_rasterrizerState.ReleaseAndGetAddressOf());
	// �G�t�F�N�g�t�@�N�g���𐶐�����
	m_effectFactory = std::make_unique<DirectX::EffectFactory>(m_device);
}


// ������`�悷��
void EditorGizmo::DrawLine(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& vector, const DirectX::FXMVECTOR& color)
{
	// ���_�J���[��ݒ肷��
	DirectX::VertexPositionColor vertex[2] =
	{
		{ DirectX::SimpleMath::Vector3(position.x, position.y, position.z), color },
		{ DirectX::SimpleMath::Vector3(position.x + vector.x, position.y + vector.y, position.z + vector.z), color }
	};
	// ������`�悷��
	m_primitiveBatch->DrawLine(vertex[0], vertex[1]);
}


// �~��`�悷��
void EditorGizmo::DrawCircle(const DirectX::SimpleMath::Vector3& center, const float& radius, const DirectX::FXMVECTOR& color, const int& split)
{
	using namespace DirectX::SimpleMath;

	// �p�x������������
	float angle = 0.0f;
	// �I�_���v�Z����
	DirectX::SimpleMath::Vector3 position1 = center + Vector3(cosf(angle), center.y, sinf(angle)) * radius;
	for (int index = 0; index < split; index++)
	{
		// �n�_��ݒ肷��
		DirectX::SimpleMath::Vector3 position0 = position1;
		// �p�x���v�Z����
		angle += DirectX::XM_2PI / (float)split;
		// �I�_���v�Z����
		position1 = center + Vector3(cosf(angle), center.y, sinf(angle)) * radius;
		// �~��`�悷��
		this->DrawLine(position0, position1 - position0, color);
	}
}

DirectX::SimpleMath::Matrix EditorGizmo::DrawManipulate(const DirectX::SimpleMath::Matrix& worldMatrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode)
{
	float view[16], projection[16], world[16];
	DirectX::SimpleMath::Matrix resultWorldMatirx;

	// TK�̃��b�p�[�N���X�̍s���z��ɕϊ�
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetViewMatrix(), view);
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetProjectionMatrix(), projection);
	EditorGizmo::MatrixToFloatArrayColumnMajor(worldMatrix, world);

	// �M�Y����`��
	ImGuizmo::Manipulate(view, projection, operation, mode, world);

	// TK�̃��b�p�[�ɖ߂�
	EditorGizmo::FloatArrayToMatrixColumnMajor(&resultWorldMatirx, world);

	return resultWorldMatirx;
}

void EditorGizmo::DrawGrid()
{

	float view[16], projection[16] , world[16];

	// TK�̃��b�p�[�N���X�̍s���z��ɕϊ�
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetViewMatrix(), view);
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetProjectionMatrix(), projection);
	EditorGizmo::MatrixToFloatArrayColumnMajor(DirectX::SimpleMath::Matrix::Identity, world);

	// ��ʃT�C�Y��ݒ�
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGuizmo::SetRect(
		mainViewport->Pos.x,
		mainViewport->Pos.y,
		mainViewport->Size.x,
		mainViewport->Size.y
	);

	// �O���b�h��`��
	ImGuizmo::DrawGrid(view, projection, world, 20);
}


// SimpleMath�� Matrix �� float[16]�z�� �ɕϊ�����
void EditorGizmo::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

// SimpleMath�� float[16]�z�� �� Matrix �ɕϊ�����
void EditorGizmo::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}
