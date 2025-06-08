#pragma once
#include "Game/Parameters/ParameterBuffers.h"
#include "Game/Particle/Particle.h"
#include "Game/EditorGizmo.h"

class Particle;
class Parameters;
class EditorGizmo;

class ParticleEmitter
{
private:

	enum class Shape
	{
		CONE,
		SPHERE,
	};

public:

	// インプットデータを取得する
	std::vector<DirectX::VertexPositionColorTexture> GetInputDatas() const { return m_inputDatas; }
	// ピクセルシェーダを取得する
	ID3D11PixelShader* GetPixelShader() const { return m_pixelShader; }

	// テクスチャを取得する
	ID3D11ShaderResourceView* GetTexture() { return m_texture; }

	// ワールド行列を取得する
	DirectX::SimpleMath::Matrix GetWorldMatrix() const { return m_worldMatrix; }
	// ワールド行列を設定する
	void SetWorldMatrix(const DirectX::SimpleMath::Matrix& world) { m_worldMatrix = world; }

	// パラメータデータを設定
	void SetParticleParameters(const ParticleParameters& parameters) { m_particleParameters = parameters; }


public:

	// コンストラクタ
	ParticleEmitter(const ParticleParameters& parametersData);
	// デストラクタ
	~ParticleEmitter() = default;

	// 初期化処理
	void Initialize();

	// 更新処理
	void Update(const float& elapsedTime);

	void Emit();

	void Play();

	void Stop();

	void DebugDraw();

	void DebugWidnow();


private:

	// ランダム生成
	float RandomFloat(float min, float max);

	// コーン状のランダム生成
	void GenerateConeEmission(
		float coneAngleDeg,
		float coneRadius,
		float coneHeight,
		bool emitFromShell,
		const DirectX::SimpleMath::Vector3& coneOrigin,
		const DirectX::SimpleMath::Vector3& coneDirection, // 必ず正規化して渡す
		DirectX::SimpleMath::Vector3& outPosition,
		DirectX::SimpleMath::Vector3& outVelocity);

	// スフィア型のランダム
	void GenerateSphereEmission(
		float sphereRadius,
		bool emitFromShell,
		const DirectX::SimpleMath::Vector3& center,
		float randomDirectionStrength,
		DirectX::SimpleMath::Vector3& outPosition,
		DirectX::SimpleMath::Vector3& outVelocity);

private:

	// パラメータ
	Parameters* m_parameters;

	// パーティクル粒子
	std::vector<std::unique_ptr<Particle>> m_particles;
	std::vector<Particle*> m_activeParticles;
	// インプットデータ
	std::vector<DirectX::VertexPositionColorTexture> m_inputDatas;
	// テクスチャ
	ID3D11ShaderResourceView* m_texture;
	// ピクセルシェーダー
	ID3D11PixelShader* m_pixelShader;

	// パーティクルパラメータ
	ParticleParameters m_particleParameters;

	Shape m_shape;

	float m_elapsedTime;
	float m_duration;

	// デバッグ描画
	std::unique_ptr<EditorGizmo> m_editorGizmo;

	// ワールド行列
	DirectX::SimpleMath::Matrix m_worldMatrix;

};