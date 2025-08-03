#include "pch.h"
#include "Game/Scene.h"
#include <imgui/imgui_internal.h>
#include "Framework/CommonResources.h"
#include "Framework/Microsoft/ReadData.h"
#include "Game/EditorGizmo.h"
#include "Framework/FileDialogUtilities.h"
#include "Game/Parameters/ParameterBuffers.h"
#include "Game/Particle/ParticleEmitter.h"


const int Scene::MAX_PARTICLE_VERTEX_COUNT = 700;

/// <summary>
/// �R���X�g���N�^
/// </summary>
Scene::Scene()
	:
	m_commonResources{},
	m_device{},
	m_context{},
	m_commonStates{},
	m_gridMatrix{},
	m_arrayGridMatrix{},
	m_cameraDistance{},
	m_mainViewMatrix{},
	m_main{},
	m_sub{},
	m_fixedViewMatrix{},
	m_particleEmitter{},
	m_particleiInputLayout{},
	m_particleConstBuffer{},
	m_particleVertexShader{},
	m_particleGeometryShader{},
	m_particlePixelShader{},
	m_particleVertexBuffer{},
	m_billboardMatrix{},
	m_textures{},
	m_parametersData{}
{
	// �C���X�^���X���擾����
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
	m_commonStates    = CommonResources::GetInstance()->GetCommonStates();
}

/// <summary>
/// ����������
/// </summary>
void Scene::Initialize()
{
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

	// �Œ�J�����̃r���[�s��쐬
	DirectX::SimpleMath::Vector3 eye(5.0f, 5.0f, 5.0f);
	DirectX::SimpleMath::Vector3 target(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 up(0.0f, 1.0f, 0.0f);
	m_fixedViewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(eye, target, up);

	// �f�o�b�O�J�����̃r���[�s��
	m_cameraDistance = 10.0f;
	eye    = { 1.0f , 1.0f, 1.0f };
	eye *= m_cameraDistance;
	target = { 0.0f, 0.0f, 0.0f };
	m_mainViewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(eye, target, up);

	// �O���b�h�̃��[���h�s��쐬
	m_gridMatrix = DirectX::SimpleMath::Matrix::Identity;
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_gridMatrix, m_arrayGridMatrix);

	// �V�F�[�_�[�A�o�b�t�@�̍쐬
	this->CreateShaderAndBuffer();


	// �p�����[�^�̃f�[�^�����[�h����
	std::ifstream file("Resources/Json/Effect.json");
	if (!file) return;

	json j;
	file >> j;

	// �����p�����[�^�̃f�[�^���擾
	m_parametersData = j.get<ParticleParameters>(); 


	// �p�[�e�B�N��������쐬�Ə���������
	m_particleEmitter = std::make_unique<ParticleEmitter>(m_parametersData);
	m_particleEmitter->Initialize();

	m_particleConstBuffer = std::make_unique<ConstantBuffer<ParticleConstBuffer>>();
	m_particleConstBuffer->Initialize(m_device);

}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="elapsedTime">�o�ߎ���</param>
void Scene::Update(const float& elapsedTime)
{
	// �p�[�e�B�N���̍X�V
	m_particleEmitter->Update(elapsedTime);
}

/// <summary>
/// �`�揈��
/// </summary>
void Scene::Render()
{
	// ���̃E�B���h�E��`��
	this->SetupMainLayout();

	// ���C���V�[���̕`��
	this->DrawMainScene();
	// �T�u�V�[���̕`��
	this->DrawSubScene();
	

	// �p�[�e�B�N���f�[�^�̕ҏW
	this->ParticleDataEditor();
}

/// <summary>
/// �I������
/// </summary>
void Scene::Finalize()
{
}


void Scene::SetupMainLayout()
{
	// ����̃E�B���h�E��`��
	ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("ParticleEditor", nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking ); // ���j���[�o�[�L��

	// ���j���[�o�[�̕`��
	this->DrawMenuBar();

	// DockSpace�ݒ�
	ImGuiID dockspacecId = ImGui::GetID("ParticleEditorDockSpace");
	ImGui::DockSpace(dockspacecId, ImVec2(0, 0),
		ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);


	// Dock�\����������
	static bool dockInit = true;
	if (dockInit)
	{
		dockInit = false;

		// �����̃h�b�N���C�A�E�g���N���A
		ImGui::DockBuilderRemoveNode(dockspacecId);
		ImGui::DockBuilderAddNode(dockspacecId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspacecId, ImVec2(1280, 720));

		ImGuiID mainId = dockspacecId;

		// �E���Ƀp�[�e�B�N���ݒ�E�B���h�E(30%)
		ImGuiID dockIdRight = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.30f, nullptr, &mainId);
		// �c����㉺�ɕ����i�V�[���r���[70%�A�R���g���[��30%�j
		ImGuiID dockIdScene = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Up, 0.75f, nullptr, &mainId);
		ImGuiID dockIdControl = mainId;

		// �e�E�B���h�E��z�u
		ImGui::DockBuilderDockWindow("Scene", dockIdScene);
		ImGui::DockBuilderDockWindow("ParticleSettings", dockIdRight);
		ImGui::DockBuilderDockWindow("Controls", dockIdControl);

		// �h�b�L���O���C�A�E�g������
		ImGui::DockBuilderFinish(dockspacecId);
	}

	ImGui::End();
}


void Scene::DrawMenuBar()
{
	// �J�X�^�����j���[�o�[�̍����ݒ�
	const float menuBarHeight = 40.0f;
	const float buttonHeight = 30.0f;
	const float buttonSpacing = 8.0f;

	// ���j���[�o�[�G���A���蓮�ō쐬
	ImVec2 menuBarPos = ImGui::GetCursorScreenPos();
	ImVec2 menuBarSize = ImVec2(ImGui::GetContentRegionAvail().x, menuBarHeight);

	// ���j���[�o�[�w�i��`��
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(menuBarPos,
		ImVec2(menuBarPos.x + menuBarSize.x, menuBarPos.y + menuBarSize.y),
		IM_COL32(45, 45, 48, 255)); // �_�[�N�O���[�w�i

	// �{�^���z�u�p�̃J�[�\���ʒu�ݒ�
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (menuBarHeight - buttonHeight) * 0.5f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonSpacing);

	// �{�^���̃X�^�C���ݒ�
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));

	// �t�@�C������{�^���Q
	if (ImGui::Button(u8"�V�K�쐬"))
	{
		// �V�K�p�[�e�B�N���쐬
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"�J��"))
	{
		// �p�[�e�B�N���t�@�C���ǂݍ���
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"�ۑ�"))
	{
		// �p�[�e�B�N���t�@�C���ۑ�
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"���O��t���ĕۑ�"))
	{
		// ���O��t���ĕۑ�
	}
	ImGui::SameLine(0, buttonSpacing);

	// �Z�p���[�^�i�c���j
	ImVec2 separatorPos = ImGui::GetCursorScreenPos();
	separatorPos.y -= 5;
	drawList->AddLine(separatorPos,
		ImVec2(separatorPos.x, separatorPos.y + buttonHeight + 10),
		IM_COL32(100, 100, 100, 255), 1.0f);
	ImGui::SameLine(0, buttonSpacing * 2);

	// �e�N�X�`������{�^��
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
	if (ImGui::Button(u8"�e�N�X�`�����J��"))
	{
		// �e�N�X�`���t�@�C���ǂݍ���
	}
	ImGui::PopStyleColor();
	ImGui::SameLine(0, buttonSpacing);

	// �ҏW����{�^���Q
	if (ImGui::Button(u8"���ɖ߂�"))
	{
		// Undo����
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"��蒼��"))
	{
		// Redo����
	}
	ImGui::SameLine(0, buttonSpacing);

	// ������̃Z�p���[�^
	separatorPos = ImGui::GetCursorScreenPos();
	separatorPos.y -= 5;
	drawList->AddLine(separatorPos,
		ImVec2(separatorPos.x, separatorPos.y + buttonHeight + 10),
		IM_COL32(100, 100, 100, 255), 1.0f);
	ImGui::SameLine(0, buttonSpacing * 2);

	// �\���؂�ւ��{�^���Q
	static bool showGrid = true;
	static bool showAxis = true;

	// �g�O���{�^���̃X�^�C��
	if (showGrid)
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
	else
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

	if (ImGui::Button(u8"�O���b�h"))
	{
		showGrid = !showGrid;
	}
	ImGui::PopStyleColor();
	ImGui::SameLine(0, buttonSpacing);

	if (showAxis)
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
	else
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

	if (ImGui::Button(u8"���\��"))
	{
		showAxis = !showAxis;
	}
	ImGui::PopStyleColor();
	ImGui::SameLine(0, buttonSpacing);

	// �E�[�Ƀw���v�{�^��
	float helpButtonWidth = 80.0f;
	float availableWidth = ImGui::GetContentRegionAvail().x;
	if (availableWidth > helpButtonWidth + buttonSpacing)
	{
		ImGui::SameLine(0, availableWidth - helpButtonWidth);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.4f, 0.2f, 1.0f));
		if (ImGui::Button(u8"�w���v"))
		{
			// �w���v�\��
		}
		ImGui::PopStyleColor();
	}

	ImGui::PopStyleVar(2); // FrameRounding, FramePadding

	// ���j���[�o�[�̍����������J�[�\����i�߂�
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (menuBarHeight - buttonHeight) * 0.5f + 5);
}




/// <summary>
/// ���C���V�[����`�悷��
/// </summary>
void Scene::DrawMainScene()
{
	// �I�t�X�N���[���p��RTV���擾
	auto rtv = m_main->GetRenderTargetView();
	auto dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// �I�t�X�N���[���`��N���A
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::DarkGray);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	// �J�������̎擾
	DirectX::SimpleMath::Matrix viewMatrix       = m_mainViewMatrix;
	DirectX::SimpleMath::Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();


	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];
	EditorGizmo::MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	EditorGizmo::MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);

	// --- ImGui�E�B���h�E���ł̕`��J�n ---
	ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_None);


	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	// ���̗̈�ł̓E�B���h�E�ړ��𖳌���
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x, screenPos.y), ImVec2(contentSize));

	// �M�Y���p�`��͈͂�ݒ�
	ImGuizmo::SetRect(screenPos.x, screenPos.y, contentSize.x, contentSize.y);
	ImGuizmo::BeginFrame();

	// �r���[�L���[�u
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImVec2 viewCubePos = ImVec2(screenPos.x + contentSize.x - 128.0f, screenPos.y);
	ImVec2 viewCubeSize = ImVec2(128, 128);
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuizmo::ViewManipulate(viewArrayMatrix, 10.0f, viewCubePos, viewCubeSize, IM_COL32(0, 0, 0, 0));

	// �O���b�h�ƃM�Y����`��
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 10.0f);


	EditorGizmo::FloatArrayToMatrixColumnMajor(&m_mainViewMatrix, viewArrayMatrix);

	this->ParticleRender(m_particleEmitter->GetWorldMatrix(), viewMatrix, projectionMatrix);

	m_commonResources->SetViewMatrix(viewMatrix);
	m_commonResources->SetProjectionMatrix(projectionMatrix);
	m_particleEmitter->DebugDraw();


	// �e�N�X�`���� ImGui �ɓ\��t��
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), contentSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}


	// �t�s����擾�i�r���[�s�� �� ���[���h�s��j
	DirectX::SimpleMath::Matrix invView = m_mainViewMatrix.Invert();
	// �J�����̈ʒu�ieye�j�́A�t�s��̕��s�ړ�����
	DirectX::SimpleMath::Vector3 eyePosition = invView.Translation();
	// �����_���Œ�
	m_mainViewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(eyePosition, { 0.0f , 0.0f ,0.0f }, DirectX::SimpleMath::Vector3::Up);


	ImGui::End();
}

/// <summary>
/// �T�u�V�[����`�悷��
/// </summary>
void Scene::DrawSubScene()
{
	// �J�������̎擾
	DirectX::SimpleMath::Matrix viewMatrix = m_mainViewMatrix;
	DirectX::SimpleMath::Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();
	float viewArrayMatrix[16], projectionArrayMatrix[16];
	EditorGizmo::MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	EditorGizmo::MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);

	// �I�t�X�N���[���p��RTV���擾
	auto rtv = m_sub->GetRenderTargetView();
	auto dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// �I�t�X�N���[���`��N���A
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::DarkGray);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	// ���q��`�悷��
	this->ParticleRender(DirectX::SimpleMath::Matrix::Identity, m_fixedViewMatrix, projectionMatrix);

	// �r���[�̕ύX
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_fixedViewMatrix, viewArrayMatrix);

	// --- ImGui�E�B���h�E���ł̕`��J�n ---
	ImGui::Begin("Scene View2", nullptr, ImGuiWindowFlags_None);

	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	// ���̗̈�ł̓E�B���h�E�ړ��𖳌���
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x, screenPos.y), ImVec2(contentSize));

	// �e�N�X�`���� ImGui �ɓ\��t��
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_sub->GetShaderResourceView(), contentSize);
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
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 5.0f);

	ImGui::End();

	// �ŏI�o�̓^�[�Q�b�g�ɖ߂�
	auto defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	auto defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);
}

/// <summary>
/// ���q��`�悷��
/// </summary>
/// <param name="worldMatrix"></param>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
void Scene::ParticleRender(const DirectX::SimpleMath::Matrix& worldMatrix, const DirectX::SimpleMath::Matrix& viewMatrix, const DirectX::SimpleMath::Matrix& projectionMatrix)
{

	ParticleConstBuffer particleConstBuffer = {};
	particleConstBuffer.worldMatrix = worldMatrix.Transpose();
	particleConstBuffer.projectionMatrix = projectionMatrix.Transpose();
	particleConstBuffer.viewMatrix = viewMatrix.Transpose();
	// �r���{�[�h�s������
	m_billboardMatrix = viewMatrix.Invert();
	m_billboardMatrix._41 = 0.0f;
	m_billboardMatrix._42 = 0.0f;
	m_billboardMatrix._43 = 0.0f;
	particleConstBuffer.billboardMatrix = m_billboardMatrix.Transpose();
	particleConstBuffer.time = DirectX::SimpleMath::Vector4::Zero;

	// �萔�o�b�t�@�̍X�V
	m_particleConstBuffer->Update(m_context, particleConstBuffer);

	//	�V�F�[�_�[�ɒ萔�o�b�t�@��n��
	ID3D11Buffer* cb[1] = { m_particleConstBuffer->GetBuffer() };
	m_context->VSSetConstantBuffers(0, 1, cb);
	m_context->GSSetConstantBuffers(0, 1, cb);
	m_context->PSSetConstantBuffers(0, 1, cb);

	//	�������`��w��		��ԃA���t�@����
	ID3D11BlendState* blendstate = m_commonStates->NonPremultiplied();
	//	�������菈��
	m_context->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);

	//	�[�x�o�b�t�@�ɏ������ݎQ�Ƃ���
	m_context->OMSetDepthStencilState(m_commonStates->DepthRead(), 0);

	//	�J�����O�͂Ȃ�
	m_context->RSSetState(m_commonStates->CullNone());

	//	�C���v�b�g���C�A�E�g�̓o�^
	m_context->IASetInputLayout(m_particleiInputLayout.Get());

	// �v���~�e�B�u�g�|���W�[��ݒ�
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// ���_�V�F�[�_�[�̐ݒ�
	m_context->VSSetShader(m_particleVertexShader.Get(), nullptr, 0);
	// �W�I���g���V�F�[�_�[�̐ݒ�
	m_context->GSSetShader(m_particleGeometryShader.Get(), nullptr, 0);
	// �s�N�Z���V�F�[�_�[�̐ݒ�
	m_context->PSSetShader(m_particlePixelShader.Get(), nullptr, 0);

	// �e�N�X�`���̐ݒ�
	ID3D11ShaderResourceView* srv = m_particleEmitter->GetTexture();
	m_context->PSSetShaderResources(0, 1, &srv);

	//	�摜�p�T���v���[�̓o�^
	ID3D11SamplerState* sampler[1] = { m_commonStates->LinearWrap() };
	m_context->PSSetSamplers(0, 1, sampler);

	// ���_�o�b�t�@�̍X�V
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_context->Map(m_particleVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy(mappedResource.pData, m_particleEmitter->GetInputDatas().data(),
		sizeof(DirectX::VertexPositionColorTexture) * m_particleEmitter->GetInputDatas().size());

	m_context->Unmap(m_particleVertexBuffer.Get(), 0);

	// ���_�o�b�t�@�̐ݒ�
	UINT stride = sizeof(DirectX::VertexPositionColorTexture);
	UINT offset = 0;
	ID3D11Buffer* buffer = m_particleVertexBuffer.Get();
	m_context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

	// �`�揈��
	m_context->Draw(static_cast<UINT>(m_particleEmitter->GetInputDatas().size()), 0);

	//	�V�F�[�_�̓o�^���������Ă���
	m_context->PSSetShader(nullptr, nullptr, 0);
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->GSSetShader(nullptr, nullptr, 0);

	// �e�N�X�`�����\�[�X�����
	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	m_context->PSSetShaderResources(0, 1, nullsrv);
}


/// <summary>
/// �p�[�e�B�N���f�[�^�̕ҏW
/// </summary>
void Scene::ParticleDataEditor()
{
	ImGui::Begin("Particle Parameters Editor");

	//// ���j���[�o�[�̊J�n
	//if (ImGui::BeginMainMenuBar())
	//{
	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Load from JSON"))
	//		{
	//			// �ǂݍ��߂��Ƃ��������
	//			if (auto parametersOpt = FileDialogUtilities::OpenJsonFile<ParticleParameters>()) {
	//				m_parametersData = *parametersOpt; 
	//			}
	//		}

	//		if (ImGui::MenuItem("Save to JSON"))
	//		{
	//			// JSON�Z�[�u����
	//			FileDialogUtilities::SaveJsonFile<ParticleParameters>(m_parametersData);
	//		}

	//		if (ImGui::MenuItem("Load Texture"))
	//		{
	//			// �e�N�X�`�����[�h����
	//			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture = FileDialogUtilities::GetLoadTexture(m_device);

	//			// nullptr�Ŗ�����Ίi�[
	//			if (texture != nullptr)
	//				// �e�N�X�`�����i�[����
	//				m_textures.push_back(std::move(texture));
	//		}

	//		ImGui::EndMenu();
	//	}
	//	ImGui::EndMainMenuBar();
	//}


	// float����
	ImGui::InputFloat("Duration", &m_parametersData.duration);
	ImGui::InputFloat("Start Delay", &m_parametersData.startDelay);
	ImGui::InputFloat("Life Time", &m_parametersData.lifeTime);
	ImGui::InputFloat("Speed", &m_parametersData.speed);
	ImGui::InputFloat("Rotation", &m_parametersData.rotation);
	ImGui::InputFloat("Gravity Modifier", &m_parametersData.gravityModifier);
	ImGui::InputFloat("Emission Rate", &m_parametersData.emissionRate);
	ImGui::InputFloat("Cone Angle", &m_parametersData.coneAngle);
	ImGui::InputFloat("Cone Radius", &m_parametersData.coneRadius);
	ImGui::InputFloat("Cone Height", &m_parametersData.coneHeight);
	ImGui::InputFloat("Sphere Radius", &m_parametersData.sphereRadius);
	ImGui::InputFloat("Sphere Rand Dir Strength", &m_parametersData.sphereRandomDirectionStrength);

	// bool �`�F�b�N
	ImGui::Checkbox("Is Looping", &m_parametersData.isLooping);
	ImGui::Checkbox("Prewarm", &m_parametersData.prewarm);
	ImGui::Checkbox("Is Playing", &m_parametersData.isPlaying);
	ImGui::Checkbox("Cone Emit From Shell", &m_parametersData.coneEmitFromShell);
	ImGui::Checkbox("Sphere Emit From Shell", &m_parametersData.sphereEmitFromShell);

	// Vector3 ����
	ImGui::InputFloat3("Start Scale", &m_parametersData.startScale.x);
	ImGui::InputFloat3("Cone Direction", &m_parametersData.coneDirection.x);
	ImGui::InputFloat3("Cone Position", &m_parametersData.conePosition.x);
	ImGui::InputFloat3("Sphere Center", &m_parametersData.sphereCenter.x);

	// Vector4 ����
	ImGui::InputFloat4("Start Color", &m_parametersData.startColor.x);

	



	// ���ݑI������Ă���e�N�X�`���C���f�b�N�X
	static int selectedTextureIndex = 0;

	// �e�N�X�`�����Ɋ�Â��čő�l�𐧌�
	const int textureCount = static_cast<int>(m_textures.size());
	if (textureCount > 0)
	{
		ImGui::Text("Texture Index");

		ImGui::SameLine();
		if (ImGui::ArrowButton("##Left", ImGuiDir_Left))
		{
			if (selectedTextureIndex > 0) --selectedTextureIndex;
		}

		ImGui::SameLine();
		ImGui::Text("%d / %d", selectedTextureIndex + 1, textureCount);

		ImGui::SameLine();
		if (ImGui::ArrowButton("##Right", ImGuiDir_Right))
		{
			if (selectedTextureIndex < textureCount - 1) ++selectedTextureIndex;
		}

		// �I�����ꂽ�e�N�X�`�����g�p
		m_particleEmitter->SetTexture(m_textures[selectedTextureIndex].Get());
	}
	else
	{
		ImGui::Text("No textures loaded.");
	}


	ImGui::End();

	// �G�~�b�^�̃f�[�^�X�V
	m_particleEmitter->SetParticleParameters(m_parametersData);
}


/// <summary>
/// �V�F�[�_�[�A�o�b�t�@�̍쐬
/// </summary>
void Scene::CreateShaderAndBuffer()
{
	std::vector<uint8_t> blob;

	// ���_�V�F�[�_�[�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_VS.cso");
	DX::ThrowIfFailed(
		m_device->CreateVertexShader(blob.data(), blob.size(), nullptr,
			m_particleVertexShader.ReleaseAndGetAddressOf())
	);
	// �C���v�b�g���C�A�E�g�쐬
	m_device->CreateInputLayout(
		DirectX::VertexPositionColorTexture::InputElements,
		DirectX::VertexPositionColorTexture::InputElementCount,
		blob.data(), blob.size(),
		m_particleiInputLayout.GetAddressOf());

	// �W�I���g���V�F�[�_�[�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_GS.cso");
	DX::ThrowIfFailed(
		m_device->CreateGeometryShader(blob.data(), blob.size(), nullptr,
			m_particleGeometryShader.ReleaseAndGetAddressOf())
	);

	// �s�N�Z���V�F�[�_�[�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_PS.cso");
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(blob.data(), blob.size(), nullptr,
			m_particlePixelShader.ReleaseAndGetAddressOf())
	);

	// �V�F�[�_�[�Ƀf�[�^��n�����߂̒��_�o�b�t�@�̍쐬
	D3D11_BUFFER_DESC desc = {};
	ZeroMemory(&desc, sizeof(desc));
	// �T�C�Y�͕K�v�Ȓ��_���ɂ���
	desc.ByteWidth = sizeof(DirectX::VertexPositionColorTexture) * MAX_PARTICLE_VERTEX_COUNT;
	// ���t���[������������Ȃ�DYNAMIC���g��
	desc.Usage = D3D11_USAGE_DYNAMIC;
	// �o�C���h�t���O��VertexBuffer
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPU���珑�����݂ł���悤�ɂ���
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// �o�b�t�@�쐬
	m_device->CreateBuffer(&desc, nullptr, &m_particleVertexBuffer);

}


