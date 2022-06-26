#pragma once

#include "WireGUID.h"
#include "Registry.h"

#include <unordered_map>
#include <filesystem>

#define CREATE_COMPONENT_GUID(guid) inline static constexpr WireGUID comp_guid = guid;

#define SERIALIZE_COMPONENT(definition, type) definition; \
inline static bool type##_reg = Wire::ComponentRegistry::Register(#type, #definition, { type::comp_guid, sizeof(type) });

/*
* PROPERTY values:
* Serializable: true/false
* Visible: true/false
* Name: string
* Category: string
* SpecialType: string
*/
#define PROPERTY(...)

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
			Unknown = 13,

			Int64 = 14,
			UInt64 = 15,
			AssetHandle = 16,
			Color3 = 17,
			Color4 = 18
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

				case ComponentRegistry::PropertyType::Int64: return sizeof(int64_t);
				case ComponentRegistry::PropertyType::UInt64: return sizeof(uint64_t);
				case ComponentRegistry::PropertyType::AssetHandle: return sizeof(uint64_t);
				case ComponentRegistry::PropertyType::Color3: return sizeof(float) * 3;
				case ComponentRegistry::PropertyType::Color4: return sizeof(float) * 4;
			}

			return 0;
		}

		struct ComponentProperty
		{
			std::string name;
			std::string category;

			bool serializable = true;
			bool visible = true;

			PropertyType type;
			uint32_t offset = 0;
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
		static std::string FindValueInString(const std::string& srcString, const std::string& key);
		static std::unordered_map<std::string, size_t> GetMemberOffsetsFromString(const std::string& srcString);
		static std::pair<PropertyType, std::string> FindNameAndTypeFromString(const std::string& srcString);
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