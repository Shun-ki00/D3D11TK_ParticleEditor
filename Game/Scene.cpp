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
/// コンストラクタ
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
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
	m_commonStates    = CommonResources::GetInstance()->GetCommonStates();
}

/// <summary>
/// 初期化処理
/// </summary>
void Scene::Initialize()
{
	// スクリーンサイズを取得する
	const RECT windowSize = m_commonResources->GetDeviceResources()->GetOutputSize();

	// デバッグカメラの作成
	m_debugCamera = std::make_unique<DebugCamera>();
	m_debugCamera->Initialize(windowSize.right, windowSize.bottom);

	// レンダーテクスチャを作成する
	m_main = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_main->SetDevice(m_device);
	m_main->SetWindow(windowSize);
	// レンダーテクスチャを作成する
	m_sub = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_R8G8B8A8_UNORM);
	m_sub->SetDevice(m_device);
	m_sub->SetWindow(windowSize);

	// グリッドのワールド行列作成
	m_gridMatrix = DirectX::SimpleMath::Matrix::Identity;
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_gridMatrix, m_arrayGridMatrix);

	// シェーダー、バッファの作成
	this->CreateShaderAndBuffer();

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

/// <summary>
/// 更新処理
/// </summary>
/// <param name="elapsedTime">経過時間</param>
void Scene::Update(const float& elapsedTime)
{
	// パーティクルの更新
	m_particleEmitter->Update(elapsedTime);

	// Sceneウィンドウに触れている時だけカメラを更新する
	if (m_sceneAllowCameraInput)
	{
		m_debugCamera->Update();
	}
}

/// <summary>
/// 描画処理
/// </summary>
void Scene::Render()
{
	// 基底のウィンドウを描画
	this->SetupMainLayout();
	// メインシーンの描画
	this->DrawMainScene();
	// パーティクルデータの編集
	this->ParticleDataEditor();
}

/// <summary>
/// 終了処理
/// </summary>
void Scene::Finalize()
{
}

/// <summary>
/// 基底のウィンドウのレイアウト設定
/// </summary>
void Scene::SetupMainLayout()
{
	// 既定のウィンドウを描画
	ImGui::SetNextWindowSize(MAIN_WINDOW_SIZE, ImGuiCond_Always);
	ImGui::SetNextWindowPos(MAIN_WINDOW_POSITION, ImGuiCond_Always);
	ImGui::Begin("ParticleEditor", nullptr,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking );

	// メニューバーの描画
	this->DrawMenuBar();

	// DockSpace設定
	ImGuiID dockspacecId = ImGui::GetID("ParticleEditorDockSpace");
	ImGui::DockSpace(dockspacecId, ImVec2(0, 0),
		ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
}


void Scene::DrawMenuBar()
{
	// カスタムメニューバーの高さ設定
	const float menuBarHeight = 40.0f;
	const float buttonHeight  = 30.0f;
	const float buttonSpacing = 8.0f;

	// メニューバーエリアを手動で作成
	ImVec2 menuBarPos = ImGui::GetCursorScreenPos();
	ImVec2 menuBarSize = ImVec2(ImGui::GetContentRegionAvail().x, menuBarHeight);

	// メニューバー背景を描画
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(menuBarPos,
		ImVec2(menuBarPos.x + menuBarSize.x, menuBarPos.y + menuBarSize.y),
		IM_COL32(45, 45, 48, 255)); // ダークグレー背景

	// ボタン配置用のカーソル位置設定
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (menuBarHeight - buttonHeight) * 0.5f);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonSpacing);

	// ボタンのスタイル設定
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));

	// ファイル操作ボタン群
	if (ImGui::Button(u8"新規作成"))
	{
		// 新規パーティクル作成
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"開く"))
	{
		// パーティクルファイル読み込み
		m_parametersData = FileDialogUtilities::OpenJsonFile<ParticleParameters>().value();
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"保存"))
	{
		// パラメータをjson型に変換
		nlohmann::json j = m_parametersData;

		// ファイルパスを指定する
		std::ofstream out(FileDialogUtilities::GetFilePath());
		// ファイルを開く
		out.is_open();
		// 整形して書き込む
		out << j.dump(4); 	
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"名前を付けて保存"))
	{
		// 名前を付けて保存
		FileDialogUtilities::SaveJsonFile<ParticleParameters>(m_parametersData);

		// 保存完了
		MessageBoxA(NULL, "保存完了",
			"Save", MB_ICONERROR);
	}
	ImGui::SameLine(0, buttonSpacing);

	// セパレータ（縦線）
	ImVec2 separatorPos = ImGui::GetCursorScreenPos();
	separatorPos.y -= 5;
	drawList->AddLine(separatorPos,
		ImVec2(separatorPos.x, separatorPos.y + buttonHeight + 10),
		IM_COL32(100, 100, 100, 255), 1.0f);
	ImGui::SameLine(0, buttonSpacing * 2);

	// テクスチャ操作ボタン
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
	if (ImGui::Button(u8"テクスチャを開く"))
	{
		// テクスチャファイル読み込み
		m_textures.push_back(FileDialogUtilities::GetLoadTexture(m_device));
	}
	ImGui::PopStyleColor();
	ImGui::SameLine(0, buttonSpacing);

	// 編集操作ボタン群
	if (ImGui::Button(u8"元に戻す"))
	{
		// Undo操作
	}
	ImGui::SameLine(0, buttonSpacing);

	if (ImGui::Button(u8"やり直し"))
	{
		// Redo操作
	}
	ImGui::SameLine(0, buttonSpacing);

	// もう一つのセパレータ
	separatorPos = ImGui::GetCursorScreenPos();
	separatorPos.y -= 5;
	drawList->AddLine(separatorPos,
		ImVec2(separatorPos.x, separatorPos.y + buttonHeight + 10),
		IM_COL32(100, 100, 100, 255), 1.0f);
	ImGui::SameLine(0, buttonSpacing * 2);

	// トグルボタン グリッド
	static bool gridValues[] = { true, true, true, true, true, true, true, true };
	size_t gridValueIndex = 0;
	ImGui::Toggle(u8"グリッド", &gridValues[gridValueIndex++], ImGuiToggleFlags_Animated);
	ImGui::SameLine(0, buttonSpacing);
	// トグルボタン 軸
	static bool axisValues[] = { true, true, true, true, true, true, true, true };
	size_t axisValueIndex = 0;
	ImGui::Toggle(u8"軸表示", &axisValues[axisValueIndex++], ImGuiToggleFlags_Animated);
	ImGui::SameLine(0, buttonSpacing);

	// グリッドのアクティブ設定を行う
	m_isGridActive = gridValues[0];
	// 軸のアクティブ設定を行う
	m_isAxisActive = axisValues[0];

	// 右端にヘルプボタン
	// ヘルプボタンの代わりにダミーのスペースでレイアウト維持
	float helpButtonWidth = 80.0f;
	float availableWidth = ImGui::GetContentRegionAvail().x;
	if (availableWidth > helpButtonWidth + buttonSpacing)
	{
		ImGui::SameLine(0, availableWidth - helpButtonWidth);
		ImGui::Dummy(ImVec2(helpButtonWidth, buttonHeight));
	}

	ImGui::PopStyleVar(2); // FrameRounding, FramePadding

	// メニューバーの高さ分だけカーソルを進める
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (menuBarHeight - buttonHeight) * 0.5f + 5);
}




/// <summary>
/// メインシーンを描画する
/// </summary>
void Scene::DrawMainScene()
{
	// オフスクリーン用のRTVを取得
	auto rtv = m_main->GetRenderTargetView();
	auto dsv = m_commonResources->GetDeviceResources()->GetDepthStencilView();

	// オフスクリーン描画クリア	
	m_context->ClearRenderTargetView(rtv, DirectX::Colors::DarkGray);
	m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &rtv, dsv);

	// カメラ情報の取得
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

	// Sceneウィンドウにフォーカスがあって、マウスがUIに奪われていない時だけカメラ入力許可
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
		// Scene内にマウスあり
		allow = true;
	}


	ImVec2 windowPos = ImGui::GetWindowPos(); // スクリーン座標
	regionMin = ImGui::GetWindowContentRegionMin(); // ローカル座標
	regionMax = ImGui::GetWindowContentRegionMax(); // ローカル座標

	// ギズモ描画のスクリーン範囲
	ImVec2 screenPos = ImVec2(windowPos.x + regionMin.x, windowPos.y + regionMin.y);
	ImVec2 screenSize = ImVec2(regionMax.x - regionMin.x, regionMax.y - regionMin.y);

	// ウィンドウ移動を無効化（ここも修正）
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), screenPos, ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y));

	// ギズモ設定
	ImGuizmo::SetRect(screenPos.x, screenPos.y, screenSize.x, screenSize.y);
	ImGuizmo::BeginFrame();
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	// ビューキューブ 
	float viewCubeArrayMatrix[16];
	EditorGizmo::MatrixToFloatArrayColumnMajor(viewMatrix, viewCubeArrayMatrix);
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImVec2 viewCubePos = ImVec2(screenPos.x + screenSize.x - 128.0f, screenPos.y);
	ImVec2 viewCubeSize = ImVec2(128, 128);	
	ImGuizmo::ViewManipulate(viewCubeArrayMatrix, 10.0f, viewCubePos, viewCubeSize, IM_COL32(0, 0, 0, 0));

	// グリッドがアクティブの場合描画する
	if(m_isGridActive)
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 10.0f);

	// 軸がアクティブの場合描画する
	m_particleEmitter->DebugDraw(m_isAxisActive);

	// さらにギズモに触れていない場合だけ許可
	if (ImGuizmo::IsOver() || ImGuizmo::IsUsing())
	{
		allow = false;
	}
	m_sceneAllowCameraInput = allow;

	// パーティクルの粒子の描画
	this->ParticleRender(m_particleEmitter->GetWorldMatrix(), viewMatrix, projectionMatrix);

	m_commonResources->SetViewMatrix(viewMatrix);
	m_commonResources->SetProjectionMatrix(projectionMatrix);


	// テクスチャを ImGui に貼り付け
	if (m_main->GetShaderResourceView())
	{
		ImGui::Image((ImTextureID)m_main->GetShaderResourceView(), screenSize);
	}
	else
	{
		ImGui::Text("RenderTexture is not ready.");
	}

	ImGui::End();

	// 最終出力ターゲットに戻す
	auto defaultRTV = m_commonResources->GetDeviceResources()->GetRenderTargetView();
	auto defaultDSV = m_commonResources->GetDeviceResources()->GetDepthStencilView();
	m_context->ClearRenderTargetView(defaultRTV, DirectX::Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(defaultDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &defaultRTV, nullptr);
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
	ImGui::SetNextWindowPos(PARTICLE_SETTINGS_WINDOW_POSITION, ImGuiCond_Always);
	ImGui::SetNextWindowSize(PARTICLE_SETTINGS_WINDOW_SIZE, ImGuiCond_Always);
	ImGui::Begin("Particle Settings", nullptr,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDocking);

	// ---- float入力 ----
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

	// ---- bool チェック ----
	ImGui::Checkbox("Is Looping", &m_parametersData.isLooping);
	ImGui::Checkbox("Prewarm", &m_parametersData.prewarm);
	ImGui::Checkbox("Is Playing", &m_parametersData.isPlaying);
	ImGui::Checkbox("Cone Emit From Shell", &m_parametersData.coneEmitFromShell);
	ImGui::Checkbox("Sphere Emit From Shell", &m_parametersData.sphereEmitFromShell);

	// ---- Vector3 入力 ----
	ImGui::Text("Start Scale");
	ImGui::InputFloat3("##StartScale", &m_parametersData.startScale.x);

	ImGui::Text("Cone Direction");
	ImGui::InputFloat3("##ConeDirection", &m_parametersData.coneDirection.x);

	ImGui::Text("Cone Position");
	ImGui::InputFloat3("##ConePosition", &m_parametersData.conePosition.x);

	ImGui::Text("Sphere Center");
	ImGui::InputFloat3("##SphereCenter", &m_parametersData.sphereCenter.x);

	// ---- Vector4 入力 ----
	ImGui::Text("Start Color");
	ImGui::InputFloat4("##StartColor", &m_parametersData.startColor.x);

	// ---- テクスチャ選択 ----
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

		// 選択されたテクスチャを使用
		m_particleEmitter->SetTexture(m_textures[selectedTextureIndex].Get());
	}
	else
	{
		ImGui::Text("No textures loaded.");
	}

	ImGui::End();

	// エミッタのデータ更新
	m_particleEmitter->SetParticleParameters(m_parametersData);
}


/// <summary>
/// シェーダー、バッファの作成
/// </summary>
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

	// ピクセルシェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Particle_PS.cso");
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(blob.data(), blob.size(), nullptr,
			m_particlePixelShader.ReleaseAndGetAddressOf())
	);

	// シェーダーにデータを渡すための頂点バッファの作成
	D3D11_BUFFER_DESC desc = {};
	ZeroMemory(&desc, sizeof(desc));
	// サイズは必要な頂点分にする
	desc.ByteWidth = sizeof(DirectX::VertexPositionColorTexture) * MAX_PARTICLE_VERTEX_COUNT;
	// 毎フレーム書き換えるならDYNAMICを使う
	desc.Usage = D3D11_USAGE_DYNAMIC;
	// バインドフラグはVertexBuffer
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// CPUから書き込みできるようにする
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// バッファ作成
	m_device->CreateBuffer(&desc, nullptr, &m_particleVertexBuffer);

}


