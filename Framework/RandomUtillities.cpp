#include "pch.h"
#include "Framework/RandomUtillities.h"
#include <random>



float RandomUtillities::RandomFloat(float min, float max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}


DirectX::SimpleMath::Vector3 RandomUtillities::GenerateConeEmissio
(
	float coneAngleDeg,          // コーンの角度
	float radius,            // コーンの半径
	float coneHeight,            // コーンの高さ（スピード）
	bool emitFromShell,
	const DirectX::SimpleMath::Vector3& coneOrigin, // コーンの原点
	DirectX::SimpleMath::Vector3& outPosition,
	DirectX::SimpleMath::Vector3& outVelocity)
{

    using namespace DirectX::SimpleMath;

    // 円周上のランダム点を取得
    float theta = RandomFloat(0.0f, DirectX::XM_2PI);
    float r = emitFromShell ? radius : RandomFloat(0.0f, radius);
    Vector3 localBase(r * cosf(theta), 0.0f, r * sinf(theta));
    outPosition = coneOrigin + localBase;

    // 中心→点の外向きベクトル
    Vector3 outward = localBase;
    outward.Normalize();

    // 上方向ベクトルを blend して coneAngle 分だけ傾ける
    float coneAngleRad = DirectX::XMConvertToRadians(coneAngleDeg);
    float blendFactor = cosf(coneAngleRad);  // blendFactor: 1.0 = 上向き, 0.0 = 水平
    Vector3 direction = Vector3::Lerp(outward, Vector3::Up, blendFactor);
    direction.Normalize();

    outVelocity = direction * coneHeight;

    return outVelocity;

}