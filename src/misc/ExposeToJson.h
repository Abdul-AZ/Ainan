#pragma once
//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class Environment;
	class ParticleSystem;
}

#define EXPOSE_CUSTOMIZER_TO_JSON friend void toJson(nlohmann::json& j, const ParticleSystem& ps, size_t objectOrder);\
								  friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);\
								  friend class ParticleSystem;
