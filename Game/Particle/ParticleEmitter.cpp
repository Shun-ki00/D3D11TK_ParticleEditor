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

    // プリミティブバッチの開始
    m_editorGizmo->DrawPrimitiveBegin();
    switch (m_shape)
    {
    case ParticleEmitter::Shape::CONE:

        // コーンを描画する
        m_editorGizmo->DrawCone(
            DirectX::SimpleMath::Vector3::Zero,
            m_particleParameters.coneRadius,
            m_particleParameters.coneHeight,
            m_particleParameters.coneAngle,
            m_worldMatrix, DirectX::Colors::Blue);

        break;
    case ParticleEmitter::Shape::SPHERE:

        // スフィアの描画
        m_editorGizmo->DrawSphere(centerSphere, m_particleParameters.sphereRadius);

        break;
    default:
        break;
    }

    // プリミティブバッチの終了
    m_editorGizmo->DrawPrimitiveEnd();

   // ギズモ操作による移動を適用し、ワールド行列を更新
   m_worldMatrix =
       m_editorGizmo->DrawManipulate(m_worldMatrix, ImGuizmo::OPERATION::TRANSLATE , ImGuizmo::MODE::WORLD);

   // ウィンドウ開始
   ImGui::Begin("ParticleEmitter");

   // プレイボタン
   ImGui::Button("Play");
   // ストップボタン
   ImGui::Button("Stop");

   static int index = 0;

   // パーティクル生成の方法を変更
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


   // ウィンドウ終了
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
        RandomUtillities::GenerateSphereEmission(
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