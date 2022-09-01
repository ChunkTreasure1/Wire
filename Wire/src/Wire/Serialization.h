#pragma once

#include "WireGUID.h"
#include "Registry.h"

#include <unordered_map>
#include <filesystem>

#define SERIALIZE_COMPONENT(type, guid) inline static constexpr WireGUID comp_guid = guid;
#define REGISTER_COMPONENT(type) inline static bool type##_reg = Wire::ComponentRegistry::Register(#type, { type::comp_guid, sizeof(type) });

namespace Wire
{
	class ComponentRegistry
	{
	public:
		struct RegistrationInfo
		{
			WireGUID guid = WireGUID::Null();
			size_t size = 0;
		};

		ComponentRegistry() = delete;

		static bool Register(const std::string& name, const RegistrationInfo& guid);
		static const std::string GetNameFromGUID(const WireGUID& aGuid);
		static const RegistrationInfo& GetRegistryDataFromName(const std::string& aName);

		static std::unordered_map<std::string, RegistrationInfo>& ComponentGUIDs();
	};

	class Serializer
	{
	public:
		Serializer() = delete;

		/*
		* First 4 bytes: the entity ID
		* Next 4 bytes: the component count
		* The rest: The components and it's data
		*/
		static void SerializeEntityToFile(EntityId aId, const Registry& aRegistry, const std::filesystem::path& aSceneFolder);
		static EntityId DeserializeEntityToRegistry(const std::filesystem::path& aPath, Registry& aRegistry);
	};
}