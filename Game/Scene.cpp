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
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

void Scene::Initialize()
{

	// カメラの作成
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280.0f, 720.0f);

	// 初期化
	m_position   = DirectX::SimpleMath::Vector3::Zero;
	m_rotation   = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Up,DirectX::XMConvertToRadians(45.0f));
	m_scale      = DirectX::SimpleMath::Vector3::One;
	m_world =
		DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	m_gridMatrix = DirectX::SimpleMath::Matrix::Identity;
	this->MatrixToFloatArrayColumnMajor(m_gridMatrix, m_arrayGridMatrix);

	// ギズモ操作の種類初期化
	m_operation = ImGuizmo::TRANSLATE;
	m_mode      = ImGuizmo::LOCAL;

	// スクリーンサイズを取得する
	const RECT windowSize = m_commonResources->GetDeviceResources()->GetOutputSize();

	// レンダーテクスチャを作成する
	m_main = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_main->SetDevice(m_device);
	m_main->SetWindow(windowSize);


	// レンダーテクスチャを作成する
	m_sub = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_sub->SetDevice(m_device);
	m_sub->SetWindow(windowSize);



}


void Scene::Update(const float& elapsedTime)
{

	// カメラの更新処理
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

	// オフスクリーン用のRTVを取得
	auto rtv = m_main->GetRenderTargetView();
	auto dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// オフスクリーン描画クリア
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::AliceBlue);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	// カメラ情報の取得
	Matrix viewMatrix = m_camera->GetViewMatrix();
	Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();

	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];
	this->MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(m_world, worldArrayMatrix);

	// --- ImGuiウィンドウ内での描画開始 ---
	ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_None);


	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	// この領域ではウィンドウ移動を無効化
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x , screenPos.y) , ImVec2(contentSize));

	// テクスチャを ImGui に貼り付け
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), contentSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}


	// ギズモ用描画範囲を設定
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList()); // ウィンドウ内に描かせる！
	ImGuizmo::SetRect(screenPos.x, screenPos.y, contentSize.x, contentSize.y);
	ImGuizmo::BeginFrame();

	// グリッドとギズモを描画
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 20.0f);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawCubes(viewArrayMatrix, projectionArrayMatrix, worldArrayMatrix, 1);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::Manipulate(viewArrayMatrix, projectionArrayMatrix, m_operation, m_mode, worldArrayMatrix);

	// ギズモ操作結果の反映
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

	// 最終出力ターゲットに戻す
	auto defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	auto defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);



	// オフスクリーン用のRTVを取得
	rtv = m_sub->GetRenderTargetView();
	dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// オフスクリーン描画クリア
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::AliceBlue);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);


	// --- ImGuiウィンドウ内での描画開始 ---
	ImGui::Begin("Scene View2", nullptr, ImGuiWindowFlags_None);


	windowPos = ImGui::GetWindowPos();
	cursorPos = ImGui::GetCursorPos();
	screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	contentSize = ImGui::GetContentRegionAvail();
	// この領域ではウィンドウ移動を無効化
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x, screenPos.y), ImVec2(contentSize));

	// テクスチャを ImGui に貼り付け
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), contentSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}


	// ギズモ用描画範囲を設定
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList()); // ウィンドウ内に描かせる！
	ImGuizmo::SetRect(screenPos.x, screenPos.y, contentSize.x, contentSize.y);
	ImGuizmo::BeginFrame();

	// グリッドとギズモを描画
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 20.0f);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::DrawCubes(viewArrayMatrix, projectionArrayMatrix, worldArrayMatrix, 1);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
	ImGuizmo::Manipulate(viewArrayMatrix, projectionArrayMatrix, m_operation, m_mode, worldArrayMatrix);

	// ギズモ操作結果の反映
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

	// 最終出力ターゲットに戻す
	defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);

}

void Scene::Finalize()
{

}

// SimpleMathの Matrix を float[16]配列 に変換する
void Scene::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

// SimpleMathの float[16]配列 を Matrix に変換する
void Scene::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}



void Scene::EditTransform()
{
	using namespace DirectX::SimpleMath;

	// キーボード切替
	if (ImGui::IsKeyPressed(ImGuiKey_T)) m_operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) m_operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) m_operation = ImGuizmo::SCALE;

	// 操作タイプUI
	if (ImGui::RadioButton(u8"位置", m_operation == ImGuizmo::TRANSLATE))
		m_operation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"回転", m_operation == ImGuizmo::ROTATE))
		m_operation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"スケール", m_operation == ImGuizmo::SCALE))
		m_operation = ImGuizmo::SCALE;

	if (m_operation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton(u8"ローカル", m_mode == ImGuizmo::LOCAL))
			m_mode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton(u8"ワールド", m_mode == ImGuizmo::WORLD))
			m_mode = ImGuizmo::WORLD;
	}

	// Manipulate された transformMat を分解して各要素に反映
	float t[3] = {m_position.x , m_position.y , m_position.z },
		  s[3] = {m_scale.x ,m_scale.y ,m_scale.z };

	// オイラー角を分解して表示用に使う（変換はしない）
	DirectX::SimpleMath::Vector3 euler = m_rotation.ToEuler(); // radians
	float r[3] = {
		DirectX::XMConvertToDegrees(euler.x),
		DirectX::XMConvertToDegrees(euler.y),
		DirectX::XMConvertToDegrees(euler.z)
	};

	// GUI 入力で数値変更可能
	
	// 表示＋編集（GUIから触られたらそのときだけ反映）
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

	
	// 各要素更新
	m_position = Vector3(t[0], t[1], t[2]);
	m_scale = Vector3(s[0], s[1], s[2]);

	// ワールド行列更新
	m_world = DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	
}