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

 
    // ���̍��W 1
    DirectX::SimpleMath::Vector3 RightPosition    = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Right * m_particleParameters.coneRadius;
    DirectX::SimpleMath::Vector3 LeftPosition     = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Left * m_particleParameters.coneRadius;
    DirectX::SimpleMath::Vector3 ForwardPosition  = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Forward * m_particleParameters.coneRadius;
    DirectX::SimpleMath::Vector3 BackwardPosition = m_particleParameters.conePosition + DirectX::SimpleMath::Vector3::Backward * m_particleParameters.coneRadius;

    // ���̍��W2
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


    // ���[���h���W�ɕύX
    /*RightPosition = DirectX::SimpleMath::Vector3::Transform(RightPosition, m_worldMatrix);
    RightPosition2 = DirectX::SimpleMath::Vector3::Transform(RightPosition2, m_worldMatrix);

    LeftPosition = DirectX::SimpleMath::Vector3::Transform(LeftPosition, m_worldMatrix);
    LeftPosition2 = DirectX::SimpleMath::Vector3::Transform(LeftPosition2, m_worldMatrix);

    ForwardPosition = DirectX::SimpleMath::Vector3::Transform(ForwardPosition, m_worldMatrix);
    ForwardPosition2 = DirectX::SimpleMath::Vector3::Transform(ForwardPosition2, m_worldMatrix);

    BackwardPosition = DirectX::SimpleMath::Vector3::Transform(BackwardPosition, m_worldMatrix);
    BackwardPosition2 = DirectX::SimpleMath::Vector3::Transform(BackwardPosition2, m_worldMatrix);*/

    // ���[���h���W
    DirectX::SimpleMath::Vector3 worldPosition = DirectX::SimpleMath::Vector3::Transform(centerCone, m_worldMatrix);
    DirectX::SimpleMath::Vector3 worldPosition2 = DirectX::SimpleMath::Vector3::Transform(centerCone + DirectX::SimpleMath::Vector3::Up * RightPosition2.y, m_worldMatrix);
    

    m_editorGizmo->DrawPrimitiveBegin();
    switch (m_shape)
    {
    case ParticleEmitter::Shape::CONE:

 
        // ���̉~
        m_editorGizmo->DrawCircle3D(0.0f, m_particleParameters.coneRadius, m_worldMatrix,DirectX::Colors::Green , 20 );

        // ���`��
        m_editorGizmo->DrawLine3D(RightPosition, RightPosition2, m_worldMatrix, DirectX::Colors::Green);
        m_editorGizmo->DrawLine3D(LeftPosition, LeftPosition2, m_worldMatrix, DirectX::Colors::Green);
        m_editorGizmo->DrawLine3D(ForwardPosition, ForwardPosition2, m_worldMatrix, DirectX::Colors::Green);
        m_editorGizmo->DrawLine3D(BackwardPosition, BackwardPosition2, m_worldMatrix, DirectX::Colors::Green);
        
        // ��̉~
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

// �����_�������̂��߂̃w���p�[�֐�
float ParticleEmitter::RandomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

// �R�[����Ƀ����_���Ȉʒu�ƕ����𐶐�����
void ParticleEmitter::GenerateConeEmission(
    float coneAngleDeg,
    float coneRadius,
    float coneHeight,
    bool emitFromShell,
    const DirectX::SimpleMath::Vector3& coneOrigin,
    const DirectX::SimpleMath::Vector3& coneDirection, // �K�����K�����ēn��
    DirectX::SimpleMath::Vector3& outPosition,
    DirectX::SimpleMath::Vector3& outVelocity)
{
    float theta = this->RandomFloat(0.0f, DirectX::XM_2PI);
    float r = emitFromShell ? coneRadius : RandomFloat(0.0f, coneRadius);
    float h = emitFromShell ? coneHeight : RandomFloat(0.0f, coneHeight);

    // �~���̃x�[�X�ł̃��[�J���ʒu
    DirectX::SimpleMath::Vector3 localPos(r * cosf(theta), h, r * sinf(theta));

    // ���[�J����Y�������� coneDirection �ɍ��킹�ĉ�]����
    DirectX::SimpleMath::Vector3 up = DirectX::SimpleMath::Vector3::UnitY;


    DirectX::SimpleMath::Vector3 axis = up.Cross(coneDirection);
    float dot = up.Dot(coneDirection);

    DirectX::SimpleMath::Quaternion rotation;

    // �N���X�ς��[���x�N�g���i���s�܂��͋t�����j�̏ꍇ�̓��ʏ���
    if (axis.LengthSquared() < 1e-6f)
    {
        if (dot > 0.9999f)
        {
            // ���������F��]�s�v
            rotation = DirectX::SimpleMath::Quaternion::Identity;
        }
        else
        {
            // �^�t�F90�x��] + �ʂ̎��Ŕ��]
            // Y���Ɛ����ȔC�ӎ���I���180�x��]
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

    // ���˕����iconeDirection + �u���j
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
    // ���ʏ�̃����_���ȒP�ʃx�N�g��
    float theta = RandomFloat(0.0f, DirectX::XM_2PI);
    float phi = acosf(RandomFloat(-1.0f, 1.0f)); // �ӂ̕��z�ɒ��Ӂi���ʋψ�j

    float x = sinf(phi) * cosf(theta);
    float y = cosf(phi);
    float z = sinf(phi) * sinf(theta);

    DirectX::SimpleMath::Vector3 direction = DirectX::SimpleMath::Vector3(x, y, z);

    float radius = emitFromShell ? sphereRadius : RandomFloat(0.0f, sphereRadius);
    outPosition = center + direction * radius;

    // �O�����ɏ����u���������x�N�g����Ԃ�
    DirectX::SimpleMath::Vector3 randomDir = direction +
        DirectX::SimpleMath::Vector3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1))
        * randomDirectionStrength;
    randomDir.Normalize();

    outVelocity = randomDir;
}