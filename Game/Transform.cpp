// ============================================
// 
// ファイル名: Transform.cpp
// 概要: Transformの階層管理とアニメーション制御
// 
// 製作者 : 清水駿希
// 
// ============================================
#include "pch.h"
#include "Game/Transform.h"


/// <summary>
/// コンストラクタ
/// </summary>
Transform::Transform()
	:
	m_parent(nullptr)
{

	// ローカル座標系を設定する
	m_localPosition = DirectX::SimpleMath::Vector3::Zero;
	m_localRotation = DirectX::SimpleMath::Quaternion::Identity;
	m_localScale    = DirectX::SimpleMath::Vector3::One;
}

/// <summary>
/// 初期化処理
/// </summary>
/// <param name="position">初期座標</param>
/// <param name="rotation">初期回転</param>
/// <param name="scale">初期スケール</param>
void Transform::Initialize(
	DirectX::SimpleMath::Vector3 position,
	DirectX::SimpleMath::Quaternion rotation,
	DirectX::SimpleMath::Vector3 scale
)
{
	// ローカル座標系を設定する
	m_localPosition = position;
	m_localRotation = rotation;
	m_localScale    = scale;
}

/// <summary>
/// 更新処理
/// </summary>
void Transform::Update()
{
	// ローカル行列を計算する
		m_worldMatrix =
		DirectX::SimpleMath::Matrix::CreateScale(m_localScale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_localRotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_localPosition);

	if (m_parent != nullptr)
	{
		// 子の行列に親の変位分を上乗せする
		m_worldMatrix *= m_parent->GetWorldMatrix() ;
	}
}

// ワールド座標を取得
DirectX::SimpleMath::Vector3 Transform::GetWorldPosition() const
{
	// ワールド行列の第4列 (41, 42, 43) が平行移動成分
	return DirectX::SimpleMath::Vector3(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
}

// ワールド回転を取得
DirectX::SimpleMath::Quaternion Transform::GetWorldRotation() const
{
	using namespace DirectX::SimpleMath;

	// スケールを除去して回転行列を取り出す
	Vector3 scale = GetWorldScale();

	Matrix rotationMatrix = m_worldMatrix;
	rotationMatrix._11 /= scale.x;
	rotationMatrix._12 /= scale.x;
	rotationMatrix._13 /= scale.x;

	rotationMatrix._21 /= scale.y;
	rotationMatrix._22 /= scale.y;
	rotationMatrix._23 /= scale.y;

	rotationMatrix._31 /= scale.z;
	rotationMatrix._32 /= scale.z;
	rotationMatrix._33 /= scale.z;

	// 回転行列からクォータニオンを作成
	return Quaternion::CreateFromRotationMatrix(rotationMatrix);
}

// ワールドスケールを取得
DirectX::SimpleMath::Vector3 Transform::GetWorldScale() const
{
	using namespace DirectX::SimpleMath;

	// 各軸のスケールを計算
	float scaleX = Vector3(m_worldMatrix._11, m_worldMatrix._12, m_worldMatrix._13).Length();
	float scaleY = Vector3(m_worldMatrix._21, m_worldMatrix._22, m_worldMatrix._23).Length();
	float scaleZ = Vector3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33).Length();

	return Vector3(scaleX, scaleY, scaleZ);
}