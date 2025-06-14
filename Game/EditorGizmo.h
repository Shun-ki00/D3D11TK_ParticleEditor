#pragma once

class CommonResources;


class EditorGizmo
{
public:

	// TK提供のラッパーから配列に変換
	static void MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat);
	// TK提供のラッパーから配列に変換
	static void FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat);

public:

	// 円の描画
	void DrawCircle(const DirectX::SimpleMath::Vector3& center, const float& radius, const DirectX::FXMVECTOR& color, const int& split);
	void DrawCircle3D(
		float height,
		float radius,
		const DirectX::SimpleMath::Matrix& world,
		const DirectX::FXMVECTOR& color,
		int split);
	// 線の描画
	void DrawLine(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& vector, const DirectX::FXMVECTOR& color);
	void DrawLine3D(
		const DirectX::SimpleMath::Vector3& localStart,
		const DirectX::SimpleMath::Vector3& localEnd,
		const DirectX::SimpleMath::Matrix& world,
		const DirectX::FXMVECTOR& color);

	// グリッドを描画
	void DrawGrid();
	// ギズモを描画
	DirectX::SimpleMath::Matrix DrawManipulate(const DirectX::SimpleMath::Matrix& worldMatrix , ImGuizmo::OPERATION operation , ImGuizmo::MODE mode);

	// スフィアを描画
	void DrawSphere(DirectX::SimpleMath::Vector3 center, float radius);
	// コーンの描画
	void DrawCone(const DirectX::SimpleMath::Vector3& position ,const float& radius,const float& height, const float& angle ,const DirectX::SimpleMath::Matrix& worldMatrix , const DirectX::FXMVECTOR& color);


	// プリミティブ描画開始
	void DrawPrimitiveBegin();
	// プリミティブ描画終了
	void DrawPrimitiveEnd();

	// 初期化する
	void Initialize();

	// コンストラクタ
	EditorGizmo();
	// デストラクタ
	~EditorGizmo() = default;

private:

	// 共有リソース
	CommonResources* m_commonResources;
	// デバイス
	ID3D11Device* m_device;
	// デバイスコンテキスト
	ID3D11DeviceContext* m_context;
	// コモンステート
	DirectX::CommonStates* m_commonStates;


	// スプライトバッチ
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	// スプライトフォント
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	// ベーシックエフェクト
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	// プリミティブバッチ
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_primitiveBatch;
	// エフェクトファクトリー
	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;
	// ラスタライザーステート
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterrizerState;
	// 入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};