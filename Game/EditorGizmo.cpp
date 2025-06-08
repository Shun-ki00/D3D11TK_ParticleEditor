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
	// デバイスコンテキストを取得する
	m_context = m_commonResources->GetDeviceResources()->GetD3DDeviceContext();
	// コモンステートを取得する
	m_commonStates = m_commonResources->GetCommonStates();

	// スプライトバッチを生成する
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_context);
	// ベーシックエフェクトを生成する
	m_basicEffect = std::make_unique<DirectX::BasicEffect>(m_device);
	// スプライトフォントを生成する
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(m_device, L"resources\\fonts\\SegoeUI_18.spritefont");
	// プリミティブバッチを生成する
	m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(m_context);
	// 頂点カラーを有効にする
	m_basicEffect->SetVertexColorEnabled(true);
	// テクスチャを無効にする
	m_basicEffect->SetTextureEnabled(false);
	void const* shaderByteCode;
	size_t byteCodeLength;
	// 頂点シェーダーを取得する
	m_basicEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
	// 入力レイアウトを生成する
	m_device->CreateInputLayout(
		DirectX::VertexPositionColor::InputElements,
		DirectX::VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		m_inputLayout.ReleaseAndGetAddressOf()
	);
	// ラスタライザーディスクリプション
	CD3D11_RASTERIZER_DESC rasterizerStateDesc(
		D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
		D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, FALSE, TRUE
	);
	// ラスタライズステートを生成する
	m_device->CreateRasterizerState(&rasterizerStateDesc, m_rasterrizerState.ReleaseAndGetAddressOf());
	// エフェクトファクトリを生成する
	m_effectFactory = std::make_unique<DirectX::EffectFactory>(m_device);
}

void EditorGizmo::DrawPrimitiveBegin()
{
	// ブレンディング状態を設定する
	m_context->OMSetBlendState(m_commonStates->Opaque(), nullptr, 0xFFFFFFFF);
	// 深度ステンシル状態を設定する
	m_context->OMSetDepthStencilState(m_commonStates->DepthNone(), 0);
	// カリングを行わない
	m_context->RSSetState(m_commonStates->CullNone());
	// ラスタライザー状態を設定する
	m_context->RSSetState(m_rasterrizerState.Get());

	// ビュー行列を設定する
	m_basicEffect->SetView(m_commonResources->GetViewMatrix());
	// プロジェクション行列を設定する
	m_basicEffect->SetProjection(m_commonResources->GetProjectionMatrix());
	// ワールド行列を設定する
	m_basicEffect->SetWorld(DirectX::SimpleMath::Matrix::Identity);
	// コンテキストを設定する
	m_basicEffect->Apply(m_context);
	// 入力レイアウトを設定する
	m_context->IASetInputLayout(m_inputLayout.Get());
	// プリミティブバッチを開始する
	m_primitiveBatch->Begin();
}

void EditorGizmo::DrawPrimitiveEnd()
{
	// プリミティブパッチを終了する
	m_primitiveBatch->End();
}


// 線分を描画する
void EditorGizmo::DrawLine(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& vector, const DirectX::FXMVECTOR& color)
{
	// 頂点カラーを設定する
	DirectX::VertexPositionColor vertex[2] =
	{
		{ DirectX::SimpleMath::Vector3(position.x, position.y, position.z), color },
		{ DirectX::SimpleMath::Vector3(position.x + vector.x, position.y + vector.y, position.z + vector.z), color }
	};
	// 線分を描画する
	m_primitiveBatch->DrawLine(vertex[0], vertex[1]);
}


void EditorGizmo::DrawLine3D(
	const DirectX::SimpleMath::Vector3& localStart,
	const DirectX::SimpleMath::Vector3& localEnd,
	const DirectX::SimpleMath::Matrix& world,
	const DirectX::FXMVECTOR& color)
{
	using namespace DirectX::SimpleMath;

	Vector3 worldStart = Vector3::Transform(localStart, world);
	Vector3 worldEnd = Vector3::Transform(localEnd, world);

	DirectX::VertexPositionColor vertex[2] =
	{
		{ worldStart, color },
		{ worldEnd,   color }
	};

	m_primitiveBatch->DrawLine(vertex[0], vertex[1]);
}


// 円を描画する
void EditorGizmo::DrawCircle(const DirectX::SimpleMath::Vector3& center, const float& radius, const DirectX::FXMVECTOR& color, const int& split)
{
	using namespace DirectX::SimpleMath;

	float angle = DirectX::XMConvertToRadians(45.0f);
	// 初期点を計算（XZ 平面）
	Vector3 position1 = center + Vector3(cosf(angle), 0.0f, sinf(angle)) * radius;

	for (int index = 0; index < split; index++)
	{
		Vector3 position0 = position1;
		angle += DirectX::XM_2PI / static_cast<float>(split);
		position1 = center + Vector3(cosf(angle), 0.0f, sinf(angle)) * radius;

		// 方向ベクトルを渡す（position0 は world 座標なので position1 も world に合わせる）
		this->DrawLine(position0, position1 - position0, color);
	}
}

// 円を描画する
void EditorGizmo::DrawCircle3D(
	float height,
	float radius,
	const DirectX::SimpleMath::Matrix& world,
	const DirectX::FXMVECTOR& color,
	int split)
{
	using namespace DirectX::SimpleMath;

	float step = DirectX::XM_2PI / static_cast<float>(split);

	// 1. 回転後の方向ベクトルを取得（world行列のZ軸 or forward）
	Vector3 forward = Vector3::TransformNormal(Vector3::UnitY, world); // Y軸がコーン方向なら
	forward.Normalize();

	// 2. 原点ベースのローカル円をまず生成し回転（Transform）
	Vector3 prevLocal = DirectX::SimpleMath::Vector3::Zero + Vector3(cosf(0.0f), 0.0f, sinf(0.0f)) * radius;
	Vector3 prevWorld = Vector3::Transform(prevLocal, world) + forward * height;

	for (int i = 1; i <= split; ++i)
	{
		float angle = step * i;

		Vector3 nextLocal = DirectX::SimpleMath::Vector3::Zero + Vector3(cosf(angle), 0.0f, sinf(angle)) * radius;
		Vector3 nextWorld = Vector3::Transform(nextLocal, world) + forward * height;

		this->DrawLine(prevWorld, nextWorld - prevWorld, color);
		prevWorld = nextWorld;
	}
}

DirectX::SimpleMath::Matrix EditorGizmo::DrawManipulate(const DirectX::SimpleMath::Matrix& worldMatrix, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode)
{
	float view[16], projection[16], world[16];
	DirectX::SimpleMath::Matrix resultWorldMatirx;

	// TKのラッパークラスの行列を配列に変換
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetViewMatrix(), view);
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetProjectionMatrix(), projection);
	EditorGizmo::MatrixToFloatArrayColumnMajor(worldMatrix, world);

	// ギズモを描画
	ImGuizmo::Manipulate(view, projection, operation, mode, world);

	// TKのラッパーに戻す
	EditorGizmo::FloatArrayToMatrixColumnMajor(&resultWorldMatirx, world);

	return resultWorldMatirx;
}

void EditorGizmo::DrawGrid()
{

	float view[16], projection[16] , world[16];

	// TKのラッパークラスの行列を配列に変換
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetViewMatrix(), view);
	EditorGizmo::MatrixToFloatArrayColumnMajor(m_commonResources->GetProjectionMatrix(), projection);
	EditorGizmo::MatrixToFloatArrayColumnMajor(DirectX::SimpleMath::Matrix::Identity, world);

	// 画面サイズを設定
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGuizmo::SetRect(
		mainViewport->Pos.x,
		mainViewport->Pos.y,
		mainViewport->Size.x,
		mainViewport->Size.y
	);

	// グリッドを描画
	ImGuizmo::DrawGrid(view, projection, world, 20);
}


void EditorGizmo::DrawSphere(DirectX::SimpleMath::Vector3 center, float radius)
{
	DirectX::BoundingSphere sphere;
	sphere.Center = center;
	sphere.Radius = radius;

	// スフィアを描画
	DX::Draw(m_primitiveBatch.get(), sphere,DirectX::Colors::Green);
}


// SimpleMathの Matrix を float[16]配列 に変換する
void EditorGizmo::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

// SimpleMathの float[16]配列 を Matrix に変換する
void EditorGizmo::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}
