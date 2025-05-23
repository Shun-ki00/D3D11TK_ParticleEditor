#include "pch.h"
#include "Game/Scene.h"
#include "Framework/Microsoft/ReadData.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include <chrono>
#include "imgui/ImGuizmo.h"
#include <imgui/imgui_internal.h>




Scene::Scene()
{
	// �C���X�^���X���擾����
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

void Scene::Initialize()
{

	// �J�����̍쐬
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280.0f, 720.0f);

	// ������
	m_position   = DirectX::SimpleMath::Vector3::Zero;
	m_rotation   = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up,DirectX::XMConvertToRadians(45.0f));
	m_scale      = DirectX::SimpleMath::Vector3::One;
	m_world =
		DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	m_gridMatrix = DirectX::SimpleMath::Matrix::Identity;
	this->MatrixToFloatArrayColumnMajor(m_gridMatrix, m_arrayGridMatrix);

	// �M�Y������̎�ޏ�����
	m_operation = ImGuizmo::TRANSLATE;
	m_mode      = ImGuizmo::LOCAL;

	// �X�N���[���T�C�Y���擾����
	const RECT windowSize = m_commonResources->GetDeviceResources()->GetOutputSize();

	// �����_�[�e�N�X�`�����쐬����
	m_main = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_main->SetDevice(m_device);
	m_main->SetWindow(windowSize);


	// �����_�[�e�N�X�`�����쐬����
	m_sub = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_sub->SetDevice(m_device);
	m_sub->SetWindow(windowSize);



}


void Scene::Update(const float& elapsedTime)
{

	// �J�����̍X�V����
	auto io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		m_camera->Update();
		
	}
	m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());

}

void Scene::Render()
{
	using namespace DirectX::SimpleMath;

	// �I�t�X�N���[���p��RTV���擾
	auto rtv = m_main->GetRenderTargetView();
	auto dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// �I�t�X�N���[���`��N���A
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::AliceBlue);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	// �J�������̎擾
	Matrix viewMatrix = m_camera->GetViewMatrix();
	Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();

	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];
	this->MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(m_world, worldArrayMatrix);

	// --- ImGui�E�B���h�E���ł̕`��J�n ---
	ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_None);


	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	// ���̗̈�ł̓E�B���h�E�ړ��𖳌���
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x , screenPos.y) , ImVec2(contentSize));

	// �e�N�X�`���� ImGui �ɓ\��t��
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), contentSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}


	// �M�Y���p�`��͈͂�ݒ�
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList()); // �E�B���h�E���ɕ`������I
	ImGuizmo::SetRect(screenPos.x, screenPos.y, contentSize.x, contentSize.y);
	ImGuizmo::BeginFrame();

	// �O���b�h�ƃM�Y����`��
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 20.0f);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawCubes(viewArrayMatrix, projectionArrayMatrix, worldArrayMatrix, 1);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::Manipulate(viewArrayMatrix, projectionArrayMatrix, m_operation, m_mode, worldArrayMatrix);

	// �M�Y�����쌋�ʂ̔��f
	Matrix newView;
	this->FloatArrayToMatrixColumnMajor(&newView, viewArrayMatrix);
	m_camera->SetViewMatrix(newView);

	this->FloatArrayToMatrixColumnMajor(&m_world, worldArrayMatrix);

	float t[3], r[3], s[3];
	ImGuizmo::DecomposeMatrixToComponents(worldArrayMatrix, t, r, s);

	auto io = ImGui::GetIO();
	if (io.WantCaptureMouse)
	{
		m_position = Vector3(t[0], t[1], t[2]);
		m_scale = Vector3(s[0], s[1], s[2]);
		m_rotation = Quaternion::CreateFromRotationMatrix(m_world);
	}

	ImGui::End();

	// �ŏI�o�̓^�[�Q�b�g�ɖ߂�
	auto defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	auto defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);



	// �I�t�X�N���[���p��RTV���擾
	rtv = m_sub->GetRenderTargetView();
	dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// �I�t�X�N���[���`��N���A
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::AliceBlue);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);


	// --- ImGui�E�B���h�E���ł̕`��J�n ---
	ImGui::Begin("Scene View2", nullptr, ImGuiWindowFlags_None);


	windowPos = ImGui::GetWindowPos();
	cursorPos = ImGui::GetCursorPos();
	screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	contentSize = ImGui::GetContentRegionAvail();
	// ���̗̈�ł̓E�B���h�E�ړ��𖳌���
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x, screenPos.y), ImVec2(contentSize));

	// �e�N�X�`���� ImGui �ɓ\��t��
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), contentSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}


	// �M�Y���p�`��͈͂�ݒ�
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList()); // �E�B���h�E���ɕ`������I
	ImGuizmo::SetRect(screenPos.x, screenPos.y, contentSize.x, contentSize.y);
	ImGuizmo::BeginFrame();

	// �O���b�h�ƃM�Y����`��
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 20.0f);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawCubes(viewArrayMatrix, projectionArrayMatrix, worldArrayMatrix, 1);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::Manipulate(viewArrayMatrix, projectionArrayMatrix, m_operation, m_mode, worldArrayMatrix);

	// �M�Y�����쌋�ʂ̔��f
	this->FloatArrayToMatrixColumnMajor(&newView, viewArrayMatrix);
	m_camera->SetViewMatrix(newView);

	this->FloatArrayToMatrixColumnMajor(&m_world, worldArrayMatrix);

	ImGuizmo::DecomposeMatrixToComponents(worldArrayMatrix, t, r, s);

	if (io.WantCaptureMouse)
	{
		m_position = Vector3(t[0], t[1], t[2]);
		m_scale = Vector3(s[0], s[1], s[2]);
		m_rotation = Quaternion::CreateFromRotationMatrix(m_world);
	}

	ImGui::End();

	// �ŏI�o�̓^�[�Q�b�g�ɖ߂�
	defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);

}

void Scene::Finalize()
{

}

// SimpleMath�� Matrix �� float[16]�z�� �ɕϊ�����
void Scene::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

// SimpleMath�� float[16]�z�� �� Matrix �ɕϊ�����
void Scene::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}



void Scene::EditTransform()
{
	using namespace DirectX::SimpleMath;

	// �L�[�{�[�h�ؑ�
	if (ImGui::IsKeyPressed(ImGuiKey_T)) m_operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) m_operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) m_operation = ImGuizmo::SCALE;

	// ����^�C�vUI
	if (ImGui::RadioButton(u8"�ʒu", m_operation == ImGuizmo::TRANSLATE))
		m_operation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"��]", m_operation == ImGuizmo::ROTATE))
		m_operation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"�X�P�[��", m_operation == ImGuizmo::SCALE))
		m_operation = ImGuizmo::SCALE;

	if (m_operation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton(u8"���[�J��", m_mode == ImGuizmo::LOCAL))
			m_mode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton(u8"���[���h", m_mode == ImGuizmo::WORLD))
			m_mode = ImGuizmo::WORLD;
	}

	// Manipulate ���ꂽ transformMat �𕪉����Ċe�v�f�ɔ��f
	float t[3] = {m_position.x , m_position.y , m_position.z },
		  s[3] = {m_scale.x ,m_scale.y ,m_scale.z };

	// �I�C���[�p�𕪉����ĕ\���p�Ɏg���i�ϊ��͂��Ȃ��j
	DirectX::SimpleMath::Vector3 euler = m_rotation.ToEuler(); // radians
	float r[3] = {
		DirectX::XMConvertToDegrees(euler.x),
		DirectX::XMConvertToDegrees(euler.y),
		DirectX::XMConvertToDegrees(euler.z)
	};

	// GUI ���͂Ő��l�ύX�\
	
	// �\���{�ҏW�iGUI����G��ꂽ�炻�̂Ƃ��������f�j
	bool changed = false;
	ImGui::InputFloat3("Position", t);
	changed |= ImGui::InputFloat3("Rotation", r);
	ImGui::InputFloat3("Scale", s);

	if (changed)
	{
		m_rotation = Quaternion::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(r[1]), // yaw
			DirectX::XMConvertToRadians(r[0]), // pitch
			DirectX::XMConvertToRadians(r[2])  // roll
		);
	}

	
	// �e�v�f�X�V
	m_position = Vector3(t[0], t[1], t[2]);
	m_scale = Vector3(s[0], s[1], s[2]);

	// ���[���h�s��X�V
	m_world = DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	
}