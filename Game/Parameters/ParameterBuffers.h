#pragma once
#include "pch.h"

using json = nlohmann::json;

// パーティクル構造体
struct ParticleParameters
{
	// float
	float duration                      = 0.0f;
	float startDelay                    = 0.0f;
	float lifeTime                      = 0.0f;
	float speed                         = 0.0f;
	float rotation                      = 0.0f;
	float gravityModifier               = 0.0f;
	float emissionRate                  = 0.0f;
	float coneAngle                     = 0.0f;
	float coneRadius                    = 0.0f;
	float coneHeight                    = 0.0f;
	float sphereRadius                  = 0.0f;
	float sphereRandomDirectionStrength = 0.0f;

	// string
	std::string texture;
	std::string shader;

	// bool
	bool isLooping           = false;
	bool prewarm             = false;
	bool isPlaying           = false;
	bool coneEmitFromShell   = false;
	bool sphereEmitFromShell = false;

	// Vector3
	DirectX::SimpleMath::Vector3 startScale    = { 1.0f, 1.0f, 1.0f };
	DirectX::SimpleMath::Vector3 coneDirection = { 0.0f, 1.0f, 0.0f };
	DirectX::SimpleMath::Vector3 conePosition  = { 0.0f, 0.0f, 0.0f };
	DirectX::SimpleMath::Vector3 sphereCenter  = { 0.0f, 0.0f, 0.0f };

	// Vector4
	DirectX::SimpleMath::Vector4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
};


inline void from_json(const json& j, ParticleParameters& p)
{
	j.at("duration").get_to(p.duration);
	j.at("startDelay").get_to(p.startDelay);
	j.at("lifeTime").get_to(p.lifeTime);
	j.at("speed").get_to(p.speed);
	j.at("rotation").get_to(p.rotation);
	j.at("gravityModifier").get_to(p.gravityModifier);
	j.at("emissionRate").get_to(p.emissionRate);
	j.at("coneAngle").get_to(p.coneAngle);
	j.at("coneRadius").get_to(p.coneRadius);
	j.at("coneHeight").get_to(p.coneHeight);
	j.at("sphereRadius").get_to(p.sphereRadius);
	j.at("sphereRandomDirectionStrength").get_to(p.sphereRandomDirectionStrength);

	j.at("texture").get_to(p.texture);
	j.at("shader").get_to(p.shader);

	j.at("isLooping").get_to(p.isLooping);
	j.at("prewarm").get_to(p.prewarm);
	j.at("isPlaying").get_to(p.isPlaying);
	j.at("coneEmitFromShell").get_to(p.coneEmitFromShell);
	j.at("sphereEmitFromShell").get_to(p.sphereEmitFromShell);

	std::vector<float> vec3;
	j.at("startScale").get_to(vec3); p.startScale = { vec3[0], vec3[1], vec3[2] };
	j.at("coneDirection").get_to(vec3); p.coneDirection = { vec3[0], vec3[1], vec3[2] };
	j.at("conePosition").get_to(vec3); p.conePosition = { vec3[0], vec3[1], vec3[2] };
	j.at("sphereCenter").get_to(vec3); p.sphereCenter = { vec3[0], vec3[1], vec3[2] };

	std::vector<float> vec4;
	j.at("startColor").get_to(vec4); p.startColor = { vec4[0], vec4[1], vec4[2], vec4[3] };
}