#pragma once

#include "WireGUID.h"
#include "Registry.h"

#include <unordered_map>
#include <filesystem>

#define SERIALIZE_COMPONENT(type, guid) inline static constexpr WireGUID comp_guid = guid;

#define REGISTER_COMPONENT(definition, type) definition; \
inline static bool type##_reg = Wire::ComponentRegistry::Register(#type, #definition, { type::comp_guid, sizeof(type) });

namespace Wire
{
	class ComponentRegistry
	{
	public:
		enum class PropertyType : uint32_t
		{
			Bool = 0,
			Int = 1,
			UInt = 2,
			Short = 3,
			UShort = 4,
			Char = 5,
			UChar = 6,
			Float = 7,
			Double = 8,
			Vector2 = 9,
			Vector3 = 10,
			Vector4 = 11,
			String = 12,
			Unknown = 13
		};

		inline static const size_t GetSizeFromType(PropertyType type)
		{
			switch (type)
			{
				case ComponentRegistry::PropertyType::Bool: return sizeof(bool);
				case ComponentRegistry::PropertyType::Int: return sizeof(int32_t);
				case ComponentRegistry::PropertyType::UInt: return sizeof(uint32_t);
				case ComponentRegistry::PropertyType::Short: return sizeof(int16_t);
				case ComponentRegistry::PropertyType::UShort: return sizeof(uint16_t);
				case ComponentRegistry::PropertyType::Char: return sizeof(int8_t);
				case ComponentRegistry::PropertyType::UChar: return sizeof(uint8_t);
				case ComponentRegistry::PropertyType::Float: return sizeof(float);
				case ComponentRegistry::PropertyType::Double: return sizeof(double);
				case ComponentRegistry::PropertyType::Vector2: return sizeof(float) * 2;
				case ComponentRegistry::PropertyType::Vector3: return sizeof(float) * 3;
				case ComponentRegistry::PropertyType::Vector4: return sizeof(float) * 4;
				case ComponentRegistry::PropertyType::String: return sizeof(std::string);
			}

			return 0;
		}

		struct ComponentProperty
		{
			std::string name;
			PropertyType type;
		};

		struct RegistrationInfo
		{
			WireGUID guid = WireGUID::Null();
			size_t size = 0;
			std::string name;

			std::vector<ComponentProperty> properties;
		};

		ComponentRegistry() = delete;

		static bool Register(const std::string& name, const std::string& definitionData, const RegistrationInfo& guid);
		static const std::string GetNameFromGUID(const WireGUID& aGuid);
		static const RegistrationInfo& GetRegistryDataFromName(const std::string& aName);
		static const RegistrationInfo& GetRegistryDataFromGUID(const WireGUID& aGuid);

		static std::unordered_map<std::string, RegistrationInfo>& ComponentGUIDs();

	private:
		static void ParseDefinition(const std::string& definitionData, RegistrationInfo& outInfo);
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