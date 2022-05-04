#pragma once

#include "GUID.h"
#include <unordered_map>

#define SERIALIZE_COMPONENT(type, guid) inline static constexpr GUID comp_guid = guid;\
										inline static bool type##_reg = Wire::ComponentRegistry::Register(###type, comp_guid); \

namespace Wire
{
	class ComponentRegistry
	{
	public:
		ComponentRegistry() = delete;

		static bool Register(const std::string& name, const GUID& guid);
		static std::unordered_map<std::string, GUID>& ComponentGUIDs();
	};
}