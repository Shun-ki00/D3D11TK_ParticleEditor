#include "pch.h"
#include "Game/Scene.h"
#include <imgui/imgui_internal.h>
#include "Framework/CommonResources.h"
#include "Framework/Microsoft/ReadData.h"
#include "Game/EditorGizmo.h"
#include "Framework/FileDialogUtilities.h"
#include "Game/Parameters/ParameterBuffers.h"
#include "Game/Particle/ParticleEmitter.h"
#include "imgui/imgui_toggle.h"


const int Scene::MAX_PARTICLE_VERTEX_COUNT = 700;

const ImVec2 Scene::MAIN_WINDOW_POSITION = { 0, 0 };
const ImVec2 Scene::MAIN_WINDOW_SIZE = { 1280, 720 };

const ImVec2 Scene::SCENE_WINDOW_POSITION = { 0, 50 };
const ImVec2 Scene::SCENE_WINDOW_SIZE     = { 958, 596 };

const ImVec2 Scene::PARTICLE_SETTINGS_WINDOW_POSITION = { 959, 50 };
const ImVec2 Scene::PARTICLE_SETTINGS_WINDOW_SIZE = { 322, 669 };

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
	m_debugCamera{},
	m_main{},
	m_sub{},
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

	// �f�o�b�O�J�����̍쐬
	m_debugCamera = std::make_unique<DebugCamera>();
	m_debugCamera->Initialize(windowSize.right, windowSize.bottom);

	// �����_�[�e�N�X�`�����쐬����
	m_main = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_main->SetDevice(m_device);
	m_main->SetWindow(windowSize);
	// �����_�[�e�N�X�`�����쐬����
	m_sub = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_sub->SetDevice(m_device);
	m_sub->SetWindow(windowSize);

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

	// Scene�E�B���h�E�ɐG��Ă��鎞�����J�������X�V����
	if (m_sceneAllowCameraInput)
	{
		m_debugCamera->Update();
	}
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
	// �p�[�e�B�N���f�[�^�̕ҏW
	this->ParticleDataEditor();
}

/// <summary>
/// �I������
/// </summary>
void Scene::Finalize()
{
}

/// <summary>
/// ���̃E�B���h�E�̃��C�A�E�g�ݒ�
/// </summary>
void Scene::SetupMainLayout()
{
	// ����̃E�B���h�E��`��
	ImGui::SetNextWindowSize(MAIN_WINDOW_SIZE, ImGuiCond_Always);
	ImGui::SetNextWindowPos(MAIN_WINDOW_POSITION, ImGuiCond_Always);
	ImGui::Begin("ParticleEditor", nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking );

	// ���j���[�o�[�̕`��
	this->DrawMenuBar();

	// DockSpace�ݒ�
	ImGuiID dockspacecId = ImGui::GetID("ParticleEditorDockSpace");
	ImGui::DockSpace(dockspacecId, ImVec2(0, 0),
		ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
}


void Scene::DrawMenuBar()
{
	// �J�X�^�����j���[�o�[�̍����ݒ�
	const float menuBarHeight = 40.0f;
	const float buttonHeight  = 30.0f;
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
		m_parametersData = FileDialogUtilities::OpenJsonFile<ParticleParameters>().value();
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"�ۑ�"))
	{
		// �p�����[�^��json�^�ɕϊ�
		nlohmann::json j = m_parametersData;

		// �t�@�C���p�X���w�肷��
		std::ofstream out(FileDialogUtilities::GetFilePath());
		// �t�@�C�����J��
		out.is_open();
		// ���`���ď�������
		out << j.dump(4); 	
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"���O��t���ĕۑ�"))
	{
		// ���O��t���ĕۑ�
		FileDialogUtilities::SaveJsonFile<ParticleParameters>(m_parametersData);

		// �ۑ�����
		MessageBoxA(NULL, "�ۑ�����",
			"Save", MB_ICONERROR);
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
		m_textures.push_back(FileDialogUtilities::GetLoadTexture(m_device));
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

	// �g�O���{�^�� �O���b�h
	static bool gridValues[] = { true, true, true, true, true, true, true, true };
	size_t gridValueIndex = 0;
	ImGui::Toggle(u8"�O���b�h", &gridValues[gridValueIndex++], ImGuiToggleFlags_Animated);
	ImGui::SameLine(0, buttonSpacing);
	// �g�O���{�^�� ��
	static bool axisValues[] = { true, true, true, true, true, true, true, true };
	size_t axisValueIndex = 0;
	ImGui::Toggle(u8"���\��", &axisValues[axisValueIndex++], ImGuiToggleFlags_Animated);
	ImGui::SameLine(0, buttonSpacing);

	// �O���b�h�̃A�N�e�B�u�ݒ���s��
	m_isGridActive = gridValues[0];
	// ���̃A�N�e�B�u�ݒ���s��
	m_isAxisActive = axisValues[0];

	// �E�[�Ƀw���v�{�^��
	// �w���v�{�^���̑���Ƀ_�~�[�̃X�y�[�X�Ń��C�A�E�g�ێ�
	float helpButtonWidth = 80.0f;
	float availableWidth = ImGui::GetContentRegionAvail().x;
	if (availableWidth > helpButtonWidth + buttonSpacing)
	{
		ImGui::SameLine(0, availableWidth - helpButtonWidth);
		ImGui::Dummy(ImVec2(helpButtonWidth, buttonHeight));
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
	DirectX::SimpleMath::Matrix viewMatrix       = m_debugCamera->GetViewMatrix();
	DirectX::SimpleMath::Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();


	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];
	EditorGizmo::MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	EditorGizmo::MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);


	ImGui::SetNextWindowPos(SCENE_WINDOW_POSITION, ImGuiCond_Always);
	ImGui::SetNextWindowSize(SCENE_WINDOW_SIZE, ImGuiCond_Always);
	ImGui::Begin("Scene", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse);

	// Scene�E�B���h�E�Ƀt�H�[�J�X�������āA�}�E�X��UI�ɒD���Ă��Ȃ��������J�������͋���
	auto io = ImGui::GetIO();
	bool allow = false;

	ImVec2 winPos = ImGui::GetWindowPos();
	ImVec2 regionMin = ImGui::GetWindowContentRegionMin();
	ImVec2 regionMax = ImGui::GetWindowContentRegionMax();
	ImVec2 gizmoPos = { winPos.x + regionMin.x, winPos.y + regionMin.y };
	ImVec2 gizmoSize = { regionMax.x - regionMin.x, regionMax.y - regionMin.y };

	ImVec2 mousePos = io.MousePos;

	bool mouseInScene =
		(mousePos.x >= gizmoPos.x && mousePos.x < gizmoPos.x + gizmoSize.x) &&
		(mousePos.y >= gizmoPos.y && mousePos.y < gizmoPos.y + gizmoSize.y);

	if (mouseInScene) {
		// Scene���Ƀ}�E�X����
		allow = true;
	}


	ImVec2 windowPos = ImGui::GetWindowPos(); // �X�N���[�����W
	regionMin = ImGui::GetWindowContentRegionMin(); // ���[�J�����W
	regionMax = ImGui::GetWindowContentRegionMax(); // ���[�J�����W

	// �M�Y���`��̃X�N���[���͈�
	ImVec2 screenPos = ImVec2(windowPos.x + regionMin.x, windowPos.y + regionMin.y);
	ImVec2 screenSize = ImVec2(regionMax.x - regionMin.x, regionMax.y - regionMin.y);

	// �E�B���h�E�ړ��𖳌����i�������C���j
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), screenPos, ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y));

	// �M�Y���ݒ�
	ImGuizmo::SetRect(screenPos.x, screenPos.y, screenSize.x, screenSize.y);
	ImGuizmo::BeginFrame();
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	// �r���[�L���[�u 
	float viewCubeArrayMatrix[16];
	EditorGizmo::MatrixToFloatArrayColumnMajor(viewMatrix, viewCubeArrayMatrix);
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImVec2 viewCubePos = ImVec2(screenPos.x + screenSize.x - 128.0f, screenPos.y);
	ImVec2 viewCubeSize = ImVec2(128, 128);	
	ImGuizmo::ViewManipulate(viewCubeArrayMatrix, 10.0f, viewCubePos, viewCubeSize, IM_COL32(0, 0, 0, 0));

	// �O���b�h���A�N�e�B�u�̏ꍇ�`�悷��
	if(m_isGridActive)
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 10.0f);

	// �����A�N�e�B�u�̏ꍇ�`�悷��
	m_particleEmitter->DebugDraw(m_isAxisActive);

	// ����ɃM�Y���ɐG��Ă��Ȃ��ꍇ��������
	if (ImGuizmo::IsOver() || ImGuizmo::IsUsing())
	{
		allow = false;
	}
	m_sceneAllowCameraInput = allow;

	// �p�[�e�B�N���̗��q�̕`��
	this->ParticleRender(m_particleEmitter->GetWorldMatrix(), viewMatrix, projectionMatrix);

	m_commonResources->SetViewMatrix(viewMatrix);
	m_commonResources->SetProjectionMatrix(projectionMatrix);


	// �e�N�X�`���� ImGui �ɓ\��t��
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), screenSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}

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
	ImGui::SetNextWindowPos(PARTICLE_SETTINGS_WINDOW_POSITION, ImGuiCond_Always);
	ImGui::SetNextWindowSize(PARTICLE_SETTINGS_WINDOW_SIZE, ImGuiCond_Always);
	ImGui::Begin("Particle Settings", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDocking);

	// ---- float���� ----
	ImGui::Text("Duration");
	ImGui::InputFloat("##Duration", &m_parametersData.duration);

	ImGui::Text("Start Delay");
	ImGui::InputFloat("##StartDelay", &m_parametersData.startDelay);

	ImGui::Text("Life Time");
	ImGui::InputFloat("##LifeTime", &m_parametersData.lifeTime);

	ImGui::Text("Speed");
	ImGui::InputFloat("##Speed", &m_parametersData.speed);

	ImGui::Text("Rotation");
	ImGui::InputFloat("##Rotation", &m_parametersData.rotation);

	ImGui::Text("Gravity Modifier");
	ImGui::InputFloat("##GravityModifier", &m_parametersData.gravityModifier);

	ImGui::Text("Emission Rate");
	ImGui::InputFloat("##EmissionRate", &m_parametersData.emissionRate);

	ImGui::Text("Cone Angle");
	ImGui::InputFloat("##ConeAngle", &m_parametersData.coneAngle);

	ImGui::Text("Cone Radius");
	ImGui::InputFloat("##ConeRadius", &m_parametersData.coneRadius);

	ImGui::Text("Cone Height");
	ImGui::InputFloat("##ConeHeight", &m_parametersData.coneHeight);

	ImGui::Text("Sphere Radius");
	ImGui::InputFloat("##SphereRadius", &m_parametersData.sphereRadius);

	ImGui::Text("Sphere Rand Dir Strength");
	ImGui::InputFloat("##SphereRandDirStrength", &m_parametersData.sphereRandomDirectionStrength);

	// ---- bool �`�F�b�N ----
	ImGui::Checkbox("Is Looping", &m_parametersData.isLooping);
	ImGui::Checkbox("Prewarm", &m_parametersData.prewarm);
	ImGui::Checkbox("Is Playing", &m_parametersData.isPlaying);
	ImGui::Checkbox("Cone Emit From Shell", &m_parametersData.coneEmitFromShell);
	ImGui::Checkbox("Sphere Emit From Shell", &m_parametersData.sphereEmitFromShell);

	// ---- Vector3 ���� ----
	ImGui::Text("Start Scale");
	ImGui::InputFloat3("##StartScale", &m_parametersData.startScale.x);

	ImGui::Text("Cone Direction");
	ImGui::InputFloat3("##ConeDirection", &m_parametersData.coneDirection.x);

	ImGui::Text("Cone Position");
	ImGui::InputFloat3("##ConePosition", &m_parametersData.conePosition.x);

	ImGui::Text("Sphere Center");
	ImGui::InputFloat3("##SphereCenter", &m_parametersData.sphereCenter.x);

	// ---- Vector4 ���� ----
	ImGui::Text("Start Color");
	ImGui::InputFloat4("##StartColor", &m_parametersData.startColor.x);

	// ---- �e�N�X�`���I�� ----
	static int selectedTextureIndex = 0;
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


