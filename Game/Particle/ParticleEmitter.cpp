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


    // �`��E�o�b�t�@�n ============================
    m_inputDatas.clear();                      // ���_�f�[�^������
    m_texture = nullptr;                       // �e�N�X�`��
    m_pixelShader = nullptr;                   // �s�N�Z���V�F�[�_
    m_shape = Shape::CONE;                     // �����`��i�C�ӂ�CONE�܂���SPHERE�j

    m_editorGizmo = std::make_unique<EditorGizmo>();
    m_editorGizmo->Initialize();

    m_worldMatrix = DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Up * 3.0f);
}


void ParticleEmitter::Update(const float& elapsedTime)
{
    if (!m_particleParameters.isPlaying)
        return;

    // �o�ߎ��Ԃ��X�V
    m_elapsedTime += elapsedTime;

    // ���[�v���Ȃ��ꍇ�Aduration �𒴂������~
    if (!m_particleParameters.isLooping && m_elapsedTime > m_duration)
    {
        Stop();
        return;
    }

    // �G�~�b�V��������
    static float emissionTimer = 0.0f;
    emissionTimer += elapsedTime;

    // �����^�C�~���O�ɒB������ Emit()
    float emitInterval = 1.0f / m_particleParameters.emissionRate;
    while (emissionTimer >= emitInterval)
    {
        Emit(); // �p�[�e�B�N���𔭐�������
        emissionTimer -= emitInterval;
    }

    m_inputDatas.clear();

    // �A�N�e�B�u�p�[�e�B�N���̍X�V
    for (auto it = m_activeParticles.begin(); it != m_activeParticles.end(); )
    {
        Particle* p = *it;
        p->Update(elapsedTime);

        if (!p->GetIsActive())
        {
            // �������s���Ĕ�A�N�e�B�u�ɂȂ����ꍇ�A���X�g����폜
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

    // �v���~�e�B�u�o�b�`�̊J�n
    m_editorGizmo->DrawPrimitiveBegin();
    switch (m_shape)
    {
    case ParticleEmitter::Shape::CONE:

        // �R�[����`�悷��
        m_editorGizmo->DrawCone(
            DirectX::SimpleMath::Vector3::Zero,
            m_particleParameters.coneRadius,
            m_particleParameters.coneHeight,
            m_particleParameters.coneAngle,
            m_worldMatrix, DirectX::Colors::Blue);

        break;
    case ParticleEmitter::Shape::SPHERE:

        // �X�t�B�A�̕`��
        m_editorGizmo->DrawSphere(centerSphere, m_particleParameters.sphereRadius);

        break;
    default:
        break;
    }

    // �v���~�e�B�u�o�b�`�̏I��
    m_editorGizmo->DrawPrimitiveEnd();

   // �M�Y������ɂ��ړ���K�p���A���[���h�s����X�V
   m_worldMatrix =
       m_editorGizmo->DrawManipulate(m_worldMatrix, ImGuizmo::OPERATION::TRANSLATE , ImGuizmo::MODE::WORLD);

   // �E�B���h�E�J�n
   ImGui::Begin("ParticleEmitter");

   // �v���C�{�^��
   ImGui::Button("Play");
   // �X�g�b�v�{�^��
   ImGui::Button("Stop");

   static int index = 0;

   // �p�[�e�B�N�������̕��@��ύX
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


   // �E�B���h�E�I��
   ImGui::End();

}


void ParticleEmitter::Emit()
{
    DirectX::SimpleMath::Vector3 outPosition;
    DirectX::SimpleMath::Vector3 outVelocity;
    switch (m_shape)
    {
    case ParticleEmitter::Shape::CONE:

        // �����_������
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

        // �����_������
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

  
    // --- ���� ---
    float lifetime = m_particleParameters.lifeTime;

    // --- �T�C�Y ---
    float startSize = m_particleParameters.startScale.x;
    float endSize = m_particleParameters.startScale.x;

    // --- �F ---
    DirectX::SimpleMath::Vector4 startColor = m_particleParameters.startColor;
    DirectX::SimpleMath::Vector4 endColor = m_particleParameters.startColor;

    // --- �����x ---
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