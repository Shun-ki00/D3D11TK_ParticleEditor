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

	// 最大粒子の数
	static const int MAX_PARTICLE_VERTEX_COUNT;

	// Sceneウィンドウの座標とサイズ
	static const ImVec2 MAIN_WINDOW_POSITION;
	static const ImVec2 MAIN_WINDOW_SIZE;

	// Sceneウィンドウの座標とサイズ
	static const ImVec2 SCENE_WINDOW_POSITION;
	static const ImVec2 SCENE_WINDOW_SIZE;

	// Particle Settingsウィンドウの座標とサイズ
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

	// コンストラクタ
	Scene();
	// デストラクタ
	~Scene() = default;

public:

	// 初期化処理
	void Initialize();
	// 更新処理
	void Update(const float& elapsedTime);
	// 描画処理
	void Render();
	// 終了処理
	void Finalize();

private:

	// シェーダーとバッファの作成
	void CreateShaderAndBuffer();
	// パーティクル描画処理
	void ParticleRender(const DirectX::SimpleMath::Matrix& worldMatrix, 
		const DirectX::SimpleMath::Matrix& viewMatrix, const DirectX::SimpleMath::Matrix& projectionMatrix);

	// パーティクルデータの編集
	void ParticleDataEditor();

	// メインシーンを描画
	void DrawMainScene();


	// ==== ImGuiウィンドウ作成 ===

	// レイアウトを設定
	void SetupMainLayout();
	// シーンウィンドウ

	// 値設定ウィンドウ

	// メニューバー
	void DrawMenuBar();

	// ボタンウィンドウ




private:

	// ==== フレームワーク ====

	// 共有リソース
	CommonResources* m_commonResources;

	// デバイス
	ID3D11Device1* m_device;
	// コンテキスト
	ID3D11DeviceContext1* m_context;
	// コモンステート
	DirectX::CommonStates* m_commonStates;


	// ==== アクティブ設定 ====

	// グリッドのアクティブ設定
	bool m_isGridActive;
	// 軸のアクティブ設定
	bool m_isAxisActive;
	

	// 原点行列
	DirectX::SimpleMath::Matrix m_gridMatrix;
	float m_arrayGridMatrix[16];

	// デバッグカメラ
	std::unique_ptr<DebugCamera> m_debugCamera;
	// ウィンドウに触れているかどうか
	bool m_sceneAllowCameraInput;

	// オフスクリーン用レンダーテクスチャ
	std::unique_ptr<DX::RenderTexture> m_main;
	// オフスクリーン用レンダーテクスチャ
	std::unique_ptr<DX::RenderTexture> m_sub;


	// === パーティクル ===

	// パーティクル生成器
	std::unique_ptr<ParticleEmitter> m_particleEmitter;
	// インプットレイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_particleiInputLayout;
	// 定数バッファ
	std::unique_ptr<ConstantBuffer<ParticleConstBuffer>> m_particleConstBuffer;
	// 頂点シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_particleVertexShader;
	// ジオメトリシェーダー
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_particleGeometryShader;
	// ピクセルリシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_particlePixelShader;
	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_particleVertexBuffer;
	// ビルボード
	DirectX::SimpleMath::Matrix m_billboardMatrix;

	// テクスチャコンテナ
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;

	// パラメータデータ
	ParticleParameters m_parametersData;

};