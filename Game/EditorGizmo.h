#pragma once

class CommonResources;


class EditorGizmo
{
public:

	// TK提供のラッパーから配列に変換
	void MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat);
	// TK提供のラッパーから配列に変換
	void FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat);

public:

	// 円の描画
	void DrawCircle(const DirectX::SimpleMath::Vector3& center, const float& radius, const DirectX::FXMVECTOR& color, const int& split);
	// 線の描画
	void DrawLine(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& vector, const DirectX::FXMVECTOR& color);

	// グリッドを描画
	void DrawGrid();

	// ギズモを描画
	DirectX::SimpleMath::Matrix DrawManipulate(const DirectX::SimpleMath::Matrix& worldMatrix , ImGuizmo::OPERATION operation , ImGuizmo::MODE mode);


	// 初期化する
	void Initialize();

	EditorGizmo();
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