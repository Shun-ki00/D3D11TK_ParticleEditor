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




void RandomUtillities::GenerateConeEmissio
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
}



void RandomUtillities::GenerateSphereEmission(
    float sphereRadius,
    bool emitFromShell,
    const DirectX::SimpleMath::Vector3& center,
    float randomDirectionStrength,
    DirectX::SimpleMath::Vector3& outPosition,
    DirectX::SimpleMath::Vector3& outVelocity)
{
    // 球面上のランダムな単位ベクトル
    float theta = RandomFloat(0.0f, DirectX::XM_2PI);
    float phi = acosf(RandomFloat(-1.0f, 1.0f)); // φの分布に注意（球面均一）

    float x = sinf(phi) * cosf(theta);
    float y = cosf(phi);
    float z = sinf(phi) * sinf(theta);

    DirectX::SimpleMath::Vector3 direction = DirectX::SimpleMath::Vector3(x, y, z);

    float radius = emitFromShell ? sphereRadius : RandomFloat(0.0f, sphereRadius);
    outPosition = center + direction * radius;

    // 外向きに少しブレた方向ベクトルを返す
    DirectX::SimpleMath::Vector3 randomDir = direction +
        DirectX::SimpleMath::Vector3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1))
        * randomDirectionStrength;
    randomDir.Normalize();

    outVelocity = randomDir;
}