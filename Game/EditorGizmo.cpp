#include "pch.h"
#include "Game/EditorGizmo.h"
#include "Framework/Microsoft/DebugDraw.h"
#include "Framework/CommonResources.h"

/// <summary>
/// コンストラクタ
/// </summary>
EditorGizmo::EditorGizmo()
	:
	m_commonResources{},
	m_device{},
	m_context{},
	m_commonStates{},
	m_spriteBatch{},
	m_spriteFont{},
	m_basicEffect{},
	m_primitiveBatch{},
	m_effectFactory{},
	m_rasterrizerState{},
	m_inputLayout{}
{
	m_commonResources = CommonResources::GetInstance();
}

/// <summary>
/// 初期化処理
/// </summary>
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

/// <summary>
/// プリミティブバッチの開始
/// </summary>
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

/// <summary>
/// プリミティブバッチの終了
/// </summary>
void EditorGizmo::DrawPrimitiveEnd()
{
	// プリミティブパッチを終了する
	m_primitiveBatch->End();
}


/// <summary>
/// 線分を描画する
/// </summary>
/// <param name="position">始点</param>
/// <param name="vector">終点</param>
/// <param name="color">カラー</param>
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


/// <summary>
///線分の描画3D
/// </summary>
/// <param name="localStart">始点</param>
/// <param name="localEnd">終点</param>
/// <param name="world">ワールド行列</param>
/// <param name="color">カラー</param>
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


/// <summary>
/// 円を描画する
/// </summary>
/// <param name="center">中心座標</param>
/// <param name="radius">半径</param>
/// <param name="color">カラー</param>
/// <param name="split"></param>
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

/// <summary>
/// 円を描画3D
/// </summary>
/// <param name="height">高さ</param>
/// <param name="radius">半径</param>
/// <param name="world">ワールド行列</param>
/// <param name="color">カラー</param>
/// <param name="split"></param>
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

/// <summary>
/// ギズモ描画
/// </summary>
/// <param name="worldMatrix">ワールド行列</param>
/// <param name="operation">オプション</param>
/// <param name="mode">モード</param>
/// <returns>変更後のワールド行列</returns>
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

/// <summary>
/// グリッド描画
/// </summary>
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


/// <summary>
/// スフィアの描画
/// </summary>
/// <param name="center">中心座標</param>
/// <param name="radius">半径</param>
void EditorGizmo::DrawSphere(DirectX::SimpleMath::Vector3 center, float radius)
{

	DirectX::BoundingSphere sphere;

	// 座標設定
	sphere.Center = center;
	// 反映設定
	sphere.Radius = radius;

	// スフィアを描画
	DX::Draw(m_primitiveBatch.get(), sphere,DirectX::Colors::Green);
}


/// <summary>
/// コーンの描画
/// </summary>
/// <param name="position">座標</param>
/// <param name="angle">角度</param>
/// <param name="worldMatrix">ワールド行列</param>
void EditorGizmo::DrawCone(const DirectX::SimpleMath::Vector3& position, const float& radius, const float& height, const float& angle, const DirectX::SimpleMath::Matrix& worldMatrix, const DirectX::FXMVECTOR& color)
{

	// === コーン底面の4方向のローカル起点 ===

	DirectX::SimpleMath::Vector3 baseRight    = position + DirectX::SimpleMath::Vector3::Right    * radius;
	DirectX::SimpleMath::Vector3 baseLeft     = position + DirectX::SimpleMath::Vector3::Left     * radius;
	DirectX::SimpleMath::Vector3 baseForward  = position + DirectX::SimpleMath::Vector3::Forward  * radius;
	DirectX::SimpleMath::Vector3 baseBackward = position + DirectX::SimpleMath::Vector3::Backward * radius;


	// === コーン高さ方向へのベクトル（仮に上方向） ===

	DirectX::SimpleMath::Vector3 coneUp = DirectX::SimpleMath::Vector3::Up * height;

	// === コーン角度に応じた回転（各方向にコーン角を付加） ===

	DirectX::SimpleMath::Quaternion rotRight    = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Forward,  DirectX::XMConvertToRadians(angle));
	DirectX::SimpleMath::Quaternion rotLeft     = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Backward, DirectX::XMConvertToRadians(angle));
	DirectX::SimpleMath::Quaternion rotForward  = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Left,     DirectX::XMConvertToRadians(angle));
	DirectX::SimpleMath::Quaternion rotBackward = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right,    DirectX::XMConvertToRadians(angle));


	// === 各方向から coneHeight 方向へ伸ばした先端点 ===

	DirectX::SimpleMath::Vector3 tipRight    = baseRight    + DirectX::SimpleMath::Vector3::Transform(coneUp, rotRight);
	DirectX::SimpleMath::Vector3 tipLeft     = baseLeft     + DirectX::SimpleMath::Vector3::Transform(coneUp, rotLeft);
	DirectX::SimpleMath::Vector3 tipForward  = baseForward  + DirectX::SimpleMath::Vector3::Transform(coneUp, rotForward);
	DirectX::SimpleMath::Vector3 tipBackward = baseBackward + DirectX::SimpleMath::Vector3::Transform(coneUp, rotBackward);


	// === 基底の円を描画する ===

	this->DrawCircle3D(0.0f, radius , worldMatrix, color, 20);

	// === 補助線の描画（ローカル座標をワールド変換で描画）===

	this->DrawLine3D(baseRight   , tipRight   , worldMatrix, color);
	this->DrawLine3D(baseLeft    , tipLeft    , worldMatrix, color);
	this->DrawLine3D(baseForward , tipForward , worldMatrix, color);
	this->DrawLine3D(baseBackward, tipBackward, worldMatrix, color);


	// === 上の円の位置・半径を取得 ===

	// Y方向の高さ（Up方向）
	float topCenterY = tipRight.y;
	// X方向の広がりを使用
	float topRadiusX = std::abs(tipRight.x);


	// === 上円を描画 ===

	this->DrawCircle3D(topCenterY, topRadiusX, worldMatrix, color, 20);

}




/// <summary>
/// SimpleMathの Matrix を float[16]配列 に変換する
/// </summary>
/// <param name="matrix">行列</param>
/// <param name="mat">配列行列</param>
void EditorGizmo::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

/// <summary>
/// SimpleMathの float[16]配列 を Matrix に変換する
/// </summary>
/// <param name="matrix">行列</param>
/// <param name="mat">配列行列</param>
void EditorGizmo::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}
