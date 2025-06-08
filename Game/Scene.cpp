#include "pch.h"
#include "Game/Scene.h"
#include "Framework/Microsoft/ReadData.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include "Game/Particle/ParticleEmitter.h"
#include "imgui/ImGuizmo.h"
#include <imgui/imgui_internal.h>
#include "Game/Parameters/ParameterBuffers.h"



Scene::Scene()
{
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
	m_commonStates    = CommonResources::GetInstance()->GetCommonStates();
}

void Scene::Initialize()
{
	// 固定カメラ作成
	DirectX::SimpleMath::Vector3 eye(5.0f, 5.0f, 5.0f);
	DirectX::SimpleMath::Vector3 target(0.0f, 0.0f, 0.0f);
	DirectX::SimpleMath::Vector3 up(0.0f, 1.0f, 0.0f);
	m_fixedViewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(eye, target, up);


	// デバッグカメラの作成
	m_cameraDistance = 10.0f;
	eye    = { 1.0f , 1.0f, 1.0f };
	eye *= m_cameraDistance;
	target = { 0.0f, 0.0f, 0.0f };
	m_mainViewMatrix = DirectX::SimpleMath::Matrix::CreateLookAt(eye, target, up);


	// グリッドのワールド行列作成
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
	m_operation = ImGuizmo::ROTATE;
	m_mode      = ImGuizmo::LOCAL;

	this->CreateShaderAndBuffer();

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


	// パラメータのデータをロードする
	std::ifstream file("Resources/Json/Effect.json");
	if (!file) return;

	json j;
	file >> j;

	// 初期パラメータのデータを取得
	m_parametersData = j.get<ParticleParameters>(); 

	// パーティクル生成器作成と初期化処理
	m_particleEmitter = std::make_unique<ParticleEmitter>(m_parametersData);
	m_particleEmitter->Initialize();

	m_particleConstBuffer = std::make_unique<ConstantBuffer<ParticleConstBuffer>>();
	m_particleConstBuffer->Initialize(m_device);

}


void Scene::Update(const float& elapsedTime)
{
	// パーティクルの更新
	m_particleEmitter->Update(elapsedTime);
}

void Scene::Render()
{
	using namespace DirectX::SimpleMath;

	// オフスクリーン用のRTVを取得
	auto rtv = m_main->GetRenderTargetView();
	auto dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// オフスクリーン描画クリア
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::DarkGray);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	

	// カメラ情報の取得
	Matrix viewMatrix = m_mainViewMatrix;
	Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();

	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];
	this->MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(m_world, worldArrayMatrix);

	ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("MainWindow", nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking);  // 親ウィンドウ自体はドッキング対象外

	// DockSpace 設定（必要なら初期分割もここで）
	ImGuiID dockspace_id = ImGui::GetID("MainWindowDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0),
		ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);

	// Dock構成を初期化（初回のみ）
	static bool dock_init = true;
	if (dock_init)
	{
		dock_init = false;
		ImGui::DockBuilderRemoveNode(dockspace_id);                   // clear
		ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(1280, 720));

		ImGuiID dock_main_id = dockspace_id;
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.75f, nullptr, &dock_main_id);
		ImGuiID dock_id_right = dock_main_id;
	}

	ImGui::End();

	// --- ImGuiウィンドウ内での描画開始 ---
	ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_None);


	ImVec2 windowPos = ImGui::GetWindowPos();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 screenPos = ImVec2(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	// この領域ではウィンドウ移動を無効化
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), ImVec2(screenPos.x , screenPos.y) , ImVec2(contentSize));

	// ギズモ用描画範囲を設定
	ImGuizmo::SetRect(screenPos.x, screenPos.y, contentSize.x, contentSize.y);
	ImGuizmo::BeginFrame();

	// ビューキューブ
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImVec2 viewCubePos = ImVec2(screenPos.x + contentSize.x - 128.0f, screenPos.y);
	ImVec2 viewCubeSize = ImVec2(128, 128);
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuizmo::ViewManipulate(viewArrayMatrix, 10.0f, viewCubePos, viewCubeSize, IM_COL32(0, 0, 0, 0));

	// グリッドとギズモを描画
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 10.0f);
	

	this->FloatArrayToMatrixColumnMajor(&m_mainViewMatrix, viewArrayMatrix);

	this->ParticleRender(m_particleEmitter->GetWorldMatrix(), viewMatrix, projectionMatrix);

	m_commonResources->SetViewMatrix(viewMatrix);
	m_commonResources->SetProjectionMatrix(projectionMatrix);
	m_particleEmitter->DebugDraw();


	// テクスチャを ImGui に貼り付け
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), contentSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}


	// 逆行列を取得（ビュー行列 → ワールド行列）
	Matrix invView = m_mainViewMatrix.Invert();
	// カメラの位置（eye）は、逆行列の平行移動部分
	Vector3 eyePosition = invView.Translation();
	// 注視点を固定
	m_mainViewMatrix = Matrix::CreateLookAt(eyePosition, {0.0f , 0.0f ,0.0f}, DirectX::SimpleMath::Vector3::Up);
	

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
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::DarkGray);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	this->ParticleRender(DirectX::SimpleMath::Matrix::Identity ,m_fixedViewMatrix, projectionMatrix);

	// ビューの変更
	this->MatrixToFloatArrayColumnMajor(m_fixedViewMatrix, viewArrayMatrix);

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
		ImGui::Image((ImTextureID)m_sub->GetShaderResourceView(), contentSize);
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
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 5.0f);

	ImGui::End();

	// 最終出力ターゲットに戻す
	defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);


	// エミッタ制御ウィンドウ
	m_particleEmitter->DebugWidnow();

	// パーティクルデータの編集
	this->ParticleDataEditor();
}

void Scene::Finalize()
{
}


/// <summary>
/// 粒子を描画する
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
	// ビルボード行列を作る
	m_billboardMatrix = viewMatrix.Invert();
	m_billboardMatrix._41 = 0.0f;
	m_billboardMatrix._42 = 0.0f;
	m_billboardMatrix._43 = 0.0f;
	particleConstBuffer.billboardMatrix = m_billboardMatrix.Transpose();
	particleConstBuffer.time = DirectX::SimpleMath::Vector4::Zero;

	// 定数バッファの更新
	m_particleConstBuffer->Update(m_context, particleConstBuffer);

	//	シェーダーに定数バッファを渡す
	ID3D11Buffer* cb[1] = { m_particleConstBuffer->GetBuffer() };
	m_context->VSSetConstantBuffers(0, 1, cb);
	m_context->GSSetConstantBuffers(0, 1, cb);
	m_context->PSSetConstantBuffers(0, 1, cb);

	//	半透明描画指定		補間アルファ合成
	ID3D11BlendState* blendstate = m_commonStates->NonPremultiplied();
	//	透明判定処理
	m_context->OMSetBlendState(blendstate, nullptr, 0xFFFFFFFF);

	//	深度バッファに書き込み参照する
	m_context->OMSetDepthStencilState(m_commonStates->DepthRead(), 0);

	//	カリングはなし
	m_context->RSSetState(m_commonStates->CullNone());

	//	インプットレイアウトの登録
	m_context->IASetInputLayout(m_particleiInputLayout.Get());

	// プリミティブトポロジーを設定
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// 頂点シェーダーの設定
	m_context->VSSetShader(m_particleVertexShader.Get(), nullptr, 0);
	// ジオメトリシェーダーの設定
	m_context->GSSetShader(m_particleGeometryShader.Get(), nullptr, 0);
	// ピクセルシェーダーの設定
	m_context->PSSetShader(m_particlePixelShader.Get(), nullptr, 0);

	// テクスチャの設定
	ID3D11ShaderResourceView* srv = m_particleEmitter->GetTexture();
	m_context->PSSetShaderResources(0, 1, &srv);

	//	画像用サンプラーの登録
	ID3D11SamplerState* sampler[1] = { m_commonStates->LinearWrap() };
	m_context->PSSetSamplers(0, 1, sampler);

	// 頂点バッファの更新
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_context->Map(m_particleVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy(mappedResource.pData, m_particleEmitter->GetInputDatas().data(),
		sizeof(DirectX::VertexPositionColorTexture) * m_particleEmitter->GetInputDatas().size());

	m_context->Unmap(m_particleVertexBuffer.Get(), 0);

	// 頂点バッファの設定
	UINT stride = sizeof(DirectX::VertexPositionColorTexture);
	UINT offset = 0;
	ID3D11Buffer* buffer = m_particleVertexBuffer.Get();
	m_context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

	// 描画処理
	m_context->Draw(static_cast<UINT>(m_particleEmitter->GetInputDatas().size()), 0);

	//	シェーダの登録を解除しておく
	m_context->PSSetShader(nullptr, nullptr, 0);
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->GSSetShader(nullptr, nullptr, 0);

	// テクスチャリソースを解放
	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	m_context->PSSetShaderResources(0, 1, nullsrv);
}


/// <summary>
/// パーティクルデータの編集
/// </summary>
void Scene::ParticleDataEditor()
{
	ImGui::Begin("Particle Parameters Editor");

	// float入力
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

	// bool チェック
	ImGui::Checkbox("Is Looping", &m_parametersData.isLooping);
	ImGui::Checkbox("Prewarm", &m_parametersData.prewarm);
	ImGui::Checkbox("Is Playing", &m_parametersData.isPlaying);
	ImGui::Checkbox("Cone Emit From Shell", &m_parametersData.coneEmitFromShell);
	ImGui::Checkbox("Sphere Emit From Shell", &m_parametersData.sphereEmitFromShell);

	// string 入力
	static char textureBuf[256];
	strncpy(textureBuf, m_parametersData.texture.c_str(), sizeof(textureBuf));
	if (ImGui::InputText("Texture", textureBuf, IM_ARRAYSIZE(textureBuf))) {
		m_parametersData.texture = textureBuf;
	}

	static char shaderBuf[256];
	strncpy(shaderBuf, m_parametersData.shader.c_str(), sizeof(shaderBuf));
	if (ImGui::InputText("Shader", shaderBuf, IM_ARRAYSIZE(shaderBuf))) {
		m_parametersData.shader = shaderBuf;
	}

	// Vector3 入力
	ImGui::InputFloat3("Start Scale", &m_parametersData.startScale.x);
	ImGui::InputFloat3("Cone Direction", &m_parametersData.coneDirection.x);
	ImGui::InputFloat3("Cone Position", &m_parametersData.conePosition.x);
	ImGui::InputFloat3("Sphere Center", &m_parametersData.sphereCenter.x);

	// Vector4 入力
	ImGui::InputFloat4("Start Color", &m_parametersData.startColor.x);

	ImGui::End();

	// エミッタのデータ更新
	m_particleEmitter->SetParticleParameters(m_parametersData);

	// エミッタのパラメータを変更
	// テクスチャの変更があった場合　テクスチャの変更

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


void Scene::CreateShaderAndBuffer()
{
	std::vector<uint8_t> blob;

	// 頂点シェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_VS.cso");
	DX::ThrowIfFailed(
		m_device->CreateVertexShader(blob.data(), blob.size(), nullptr,
			m_particleVertexShader.ReleaseAndGetAddressOf())
	);
	// インプットレイアウト作成
	m_device->CreateInputLayout(
		DirectX::VertexPositionColorTexture::InputElements,
		DirectX::VertexPositionColorTexture::InputElementCount,
		blob.data(), blob.size(),
		m_particleiInputLayout.GetAddressOf());

	// ジオメトリシェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_GS.cso");
	DX::ThrowIfFailed(
		m_device->CreateGeometryShader(blob.data(), blob.size(), nullptr,
			m_particleGeometryShader.ReleaseAndGetAddressOf())
	);

	// シェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_PS.cso");
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(blob.data(), blob.size(), nullptr,
			m_particlePixelShader.ReleaseAndGetAddressOf())
	);

	// シェーダーにデータを渡すための頂点バッファの作成
	D3D11_BUFFER_DESC desc = {};
	ZeroMemory(&desc, sizeof(desc));
	// サイズは必要な頂点分にする
	desc.ByteWidth = sizeof(DirectX::VertexPositionColorTexture) * 700;
	// 毎フレーム書き換えるならDYNAMICを使う
	desc.Usage = D3D11_USAGE_DYNAMIC;
	// バインドフラグはVertexBuffer
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUから書き込みできるようにする
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// バッファ作成
	m_device->CreateBuffer(&desc, nullptr, &m_particleVertexBuffer);

}


