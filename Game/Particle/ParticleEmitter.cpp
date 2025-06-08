#include "pch.h"
#include "Game/Particle/ParticleEmitter.h"
#include "Framework/CommonResources.h"
#include "Game/Particle/Particle.h"
#include "Game/EditorGizmo.h"
#include <random>
#include "Framework/RandomUtillities.h"

ParticleEmitter::ParticleEmitter(const ParticleParameters& parametersData)
{
    m_particleParameters = parametersData;
}

void ParticleEmitter::Initialize()
{

    const size_t poolSize = 256;
    m_particles.reserve(poolSize);
    for (size_t i = 0; i < poolSize; ++i) {
        m_particles.emplace_back(std::make_unique<Particle>());
    }


    // 描画・バッファ系 ============================
    m_inputDatas.clear();                      // 頂点データ初期化
    m_texture = nullptr;                       // テクスチャ
    m_pixelShader = nullptr;                   // ピクセルシェーダ
    m_shape = Shape::CONE;                     // 初期形状（任意にCONEまたはSPHERE）

    m_editorGizmo = std::make_unique<EditorGizmo>();
    m_editorGizmo->Initialize();

    m_worldMatrix = DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Up * 3.0f);
}


void ParticleEmitter::Update(const float& elapsedTime)
{
    if (!m_particleParameters.isPlaying)
        return;

    // 経過時間を更新
    m_elapsedTime += elapsedTime;

    // ループしない場合、duration を超えたら停止
    if (!m_particleParameters.isLooping && m_elapsedTime > m_duration)
    {
        Stop();
        return;
    }

    // エミッション処理
    static float emissionTimer = 0.0f;
    emissionTimer += elapsedTime;

    // 発生タイミングに達したら Emit()
    float emitInterval = 1.0f / m_particleParameters.emissionRate;
    while (emissionTimer >= emitInterval)
    {
        Emit(); // パーティクルを発生させる
        emissionTimer -= emitInterval;
    }

    m_inputDatas.clear();

    // アクティブパーティクルの更新
    for (auto it = m_activeParticles.begin(); it != m_activeParticles.end(); )
    {
        Particle* p = *it;
        p->Update(elapsedTime);

        if (!p->GetIsActive())
        {
            // 寿命が尽きて非アクティブになった場合、リストから削除
            it = m_activeParticles.erase(it);
        }
        else
        {
            m_inputDatas.push_back({ p->GetPosition(),p->GetColor(), DirectX::SimpleMath::Vector2::Zero });
            ++it;
        }
    }

}


void ParticleEmitter::Play()
{
 
}


void ParticleEmitter::Stop()
{
  
}

void ParticleEmitter::DebugDraw()
{

    DirectX::SimpleMath::Vector3 centerSphere = DirectX::SimpleMath::Vector3::Transform(m_particleParameters.sphereCenter, m_worldMatrix);
    DirectX::SimpleMath::Vector3 centerCone = DirectX::SimpleMath::Vector3::Transform(m_particleParameters.conePosition, m_worldMatrix);

 
    // 線の座標 1
    DirectX::SimpleMath::Vector3 RightPosition    = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Right * m_particleParameters.coneRadius;
    DirectX::SimpleMath::Vector3 LeftPosition     = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Left * m_particleParameters.coneRadius;
    DirectX::SimpleMath::Vector3 ForwardPosition  = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Forward * m_particleParameters.coneRadius;
    DirectX::SimpleMath::Vector3 BackwardPosition = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Backward * m_particleParameters.coneRadius;

    // 線の座標2
    DirectX::SimpleMath::Vector3 RightPosition2 = RightPosition + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Up * m_particleParameters.coneHeight,
        DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Forward, DirectX::XMConvertToRadians(m_particleParameters.coneAngle)));
    DirectX::SimpleMath::Vector3 LeftPosition2 = LeftPosition + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Up * m_particleParameters.coneHeight,
        DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Backward, DirectX::XMConvertToRadians(m_particleParameters.coneAngle)));
    DirectX::SimpleMath::Vector3 ForwardPosition2 = ForwardPosition + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Up * m_particleParameters.coneHeight,
        DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Left, DirectX::XMConvertToRadians(m_particleParameters.coneAngle)));
    DirectX::SimpleMath::Vector3 BackwardPosition2 = BackwardPosition + DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Up * m_particleParameters.coneHeight,
        DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XMConvertToRadians(m_particleParameters.coneAngle)));

    float circle2 = RightPosition2.y;
    float circle2Center =  RightPosition2.x;


    // ワールド座標に変更
    /*RightPosition = DirectX::SimpleMath::Vector3::Transform(RightPosition, m_worldMatrix);
    RightPosition2 = DirectX::SimpleMath::Vector3::Transform(RightPosition2, m_worldMatrix);

    LeftPosition = DirectX::SimpleMath::Vector3::Transform(LeftPosition, m_worldMatrix);
    LeftPosition2 = DirectX::SimpleMath::Vector3::Transform(LeftPosition2, m_worldMatrix);

    ForwardPosition = DirectX::SimpleMath::Vector3::Transform(ForwardPosition, m_worldMatrix);
    ForwardPosition2 = DirectX::SimpleMath::Vector3::Transform(ForwardPosition2, m_worldMatrix);

    BackwardPosition = DirectX::SimpleMath::Vector3::Transform(BackwardPosition, m_worldMatrix);
    BackwardPosition2 = DirectX::SimpleMath::Vector3::Transform(BackwardPosition2, m_worldMatrix);*/

    // ワールド座標
    DirectX::SimpleMath::Vector3 worldPosition = DirectX::SimpleMath::Vector3::Transform(centerCone, m_worldMatrix);
    DirectX::SimpleMath::Vector3 worldPosition2 = DirectX::SimpleMath::Vector3::Transform(centerCone + DirectX::SimpleMath::Vector3::Up * RightPosition2.y, m_worldMatrix);
    

    m_editorGizmo->DrawPrimitiveBegin();
    switch (m_shape)
    {
    case ParticleEmitter::Shape::CONE:

 
        // 基底の円
        m_editorGizmo->DrawCircle3D(0.0f, m_particleParameters.coneRadius, m_worldMatrix,DirectX::Colors::Green , 20 );

        // 線描画
        m_editorGizmo->DrawLine3D(RightPosition, RightPosition2, m_worldMatrix, DirectX::Colors::Green);
        m_editorGizmo->DrawLine3D(LeftPosition, LeftPosition2, m_worldMatrix, DirectX::Colors::Green);
        m_editorGizmo->DrawLine3D(ForwardPosition, ForwardPosition2, m_worldMatrix, DirectX::Colors::Green);
        m_editorGizmo->DrawLine3D(BackwardPosition, BackwardPosition2, m_worldMatrix, DirectX::Colors::Green);
        
        // 上の円
        m_editorGizmo->DrawCircle3D(circle2, circle2Center, m_worldMatrix, DirectX::Colors::Green, 20);

        break;
    case ParticleEmitter::Shape::SPHERE:

        m_editorGizmo->DrawSphere(centerSphere, m_particleParameters.sphereRadius);

        break;
    default:
        break;
    }
    m_editorGizmo->DrawPrimitiveEnd();

   m_worldMatrix =
       m_editorGizmo->DrawManipulate(m_worldMatrix, ImGuizmo::OPERATION::ROTATE   , ImGuizmo::MODE::LOCAL);


}

void ParticleEmitter::DebugWidnow()
{
    ImGui::Begin("ParticleEmitter");

    ImGui::Button("Play");
    ImGui::Button("Stop");

    static int index = 0;

    std::string cone = "CONE";
    std::string Sphere = "SPHERE";

    std::vector<const char*> catItems;
    catItems.push_back(cone.c_str());
    catItems.push_back(Sphere.c_str());

    ImGui::Combo("Category", &index, catItems.data(), catItems.size());

    if (index == 0)
    {
        m_shape = Shape::CONE;
    }
    else
    {
        m_shape = Shape::SPHERE;
    }

    ImGui::End();
}


void ParticleEmitter::Emit()
{
    DirectX::SimpleMath::Vector3 outPosition;
    DirectX::SimpleMath::Vector3 outVelocity;
    switch (m_shape)
    {
    case ParticleEmitter::Shape::CONE:

        // ランダム生成
        RandomUtillities::GenerateConeEmissio(
            m_particleParameters.coneAngle,
            m_particleParameters.coneRadius,
            m_particleParameters.coneHeight,
            m_particleParameters.coneEmitFromShell,
            m_particleParameters.conePosition,
            outPosition,
            outVelocity
        );

        break;
    case ParticleEmitter::Shape::SPHERE:

        // ランダム生成
        this->GenerateSphereEmission(
            m_particleParameters.sphereRadius,
            m_particleParameters.sphereEmitFromShell,
            m_particleParameters.sphereCenter,
            m_particleParameters.sphereRandomDirectionStrength,
            outPosition,
            outVelocity
        );

        break;
    default:
        break;
    }

  
    // --- 寿命 ---
    float lifetime = m_particleParameters.lifeTime;

    // --- サイズ ---
    float startSize = m_particleParameters.startScale.x;
    float endSize = m_particleParameters.startScale.x;

    // --- 色 ---
    DirectX::SimpleMath::Vector4 startColor = m_particleParameters.startColor;
    DirectX::SimpleMath::Vector4 endColor = m_particleParameters.startColor;

    // --- 加速度 ---
    DirectX::SimpleMath::Vector3 acceleration = DirectX::SimpleMath::Vector3::Zero;

    for (auto& p : m_particles)
    {
        if (!p->GetIsActive()) 
        {
            p->Initialize(
                outPosition,
                outVelocity,
                acceleration,
                lifetime,
                startSize,
                endSize,
                startColor,
                endColor
            );
            m_activeParticles.push_back(p.get());
            break;
        }
    }
}

// ランダム生成のためのヘルパー関数
float ParticleEmitter::RandomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

// コーン状にランダムな位置と方向を生成する
void ParticleEmitter::GenerateConeEmission(
    float coneAngleDeg,
    float coneRadius,
    float coneHeight,
    bool emitFromShell,
    const DirectX::SimpleMath::Vector3& coneOrigin,
    const DirectX::SimpleMath::Vector3& coneDirection, // 必ず正規化して渡す
    DirectX::SimpleMath::Vector3& outPosition,
    DirectX::SimpleMath::Vector3& outVelocity)
{
    float theta = this->RandomFloat(0.0f, DirectX::XM_2PI);
    float r = emitFromShell ? coneRadius : RandomFloat(0.0f, coneRadius);
    float h = emitFromShell ? coneHeight : RandomFloat(0.0f, coneHeight);

    // 円錐のベースでのローカル位置
    DirectX::SimpleMath::Vector3 localPos(r * cosf(theta), h, r * sinf(theta));

    // ローカルのY軸方向を coneDirection に合わせて回転する
    DirectX::SimpleMath::Vector3 up = DirectX::SimpleMath::Vector3::UnitY;


    DirectX::SimpleMath::Vector3 axis = up.Cross(coneDirection);
    float dot = up.Dot(coneDirection);

    DirectX::SimpleMath::Quaternion rotation;

    // クロス積がゼロベクトル（平行または逆向き）の場合の特別処理
    if (axis.LengthSquared() < 1e-6f)
    {
        if (dot > 0.9999f)
        {
            // 同じ方向：回転不要
            rotation = DirectX::SimpleMath::Quaternion::Identity;
        }
        else
        {
            // 真逆：90度回転 + 別の軸で反転
            // Y軸と垂直な任意軸を選んで180度回転
            rotation = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(DirectX::SimpleMath::Vector3::Right, DirectX::XM_PI);
        }
    }
    else
    {
        axis.Normalize();
        float angle = acosf(dot);
        rotation = DirectX::SimpleMath::Quaternion::CreateFromAxisAngle(axis, angle);
    }


    DirectX::SimpleMath::Vector3 worldPos = DirectX::SimpleMath::Vector3::Transform(localPos, rotation) + coneOrigin;

    // 発射方向（coneDirection + ブレ）
    float spreadRad = DirectX::XMConvertToRadians(coneAngleDeg);
    DirectX::SimpleMath::Vector3 dir = DirectX::SimpleMath::Vector3::Transform(
        DirectX::SimpleMath::Vector3(0, 1, 0),
        DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(
            RandomFloat(-spreadRad, spreadRad), 
            RandomFloat(-spreadRad, spreadRad), 0));
    DirectX::SimpleMath::Vector3 velocity = DirectX::SimpleMath::Vector3::Transform(dir, rotation);

    outPosition = worldPos;
    outVelocity = velocity;
}

void ParticleEmitter::GenerateSphereEmission(
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