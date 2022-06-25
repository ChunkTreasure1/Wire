#include "Serialization.h"

#include <fstream>
#include <string>

namespace Wire
{
	namespace Utility
	{
		static ComponentRegistry::PropertyType PropertyFromString(const std::string& string)
		{
			if (string == "std::string") return ComponentRegistry::PropertyType::String;
			if (string == "bool") return ComponentRegistry::PropertyType::Bool;

			if (string == "int") return ComponentRegistry::PropertyType::Int;
			if (string == "int32_t") return ComponentRegistry::PropertyType::Int;

			if (string == "unsigned int") return ComponentRegistry::PropertyType::UInt; // TODO: Implement this
			if (string == "uint32_t") return ComponentRegistry::PropertyType::UInt;

			if (string == "short") return ComponentRegistry::PropertyType::Short;
			if (string == "int16_t") return ComponentRegistry::PropertyType::Short;

			if (string == "unsigned short") return ComponentRegistry::PropertyType::UShort; // TODO: Implement this
			if (string == "uint16_t") return ComponentRegistry::PropertyType::UShort;

			if (string == "char") return ComponentRegistry::PropertyType::Char;
			if (string == "int8_t") return ComponentRegistry::PropertyType::Char;

			if (string == "unsigned char") return ComponentRegistry::PropertyType::UChar; // TODO: Implement this
			if (string == "uint8_t") return ComponentRegistry::PropertyType::UChar;

			if (string == "float") return ComponentRegistry::PropertyType::Float;
			if (string == "double") return ComponentRegistry::PropertyType::Double;

			if (string == "glm::vec2") return ComponentRegistry::PropertyType::Vector2;
			if (string == "glm::vec3") return ComponentRegistry::PropertyType::Vector3;
			if (string == "glm::vec4") return ComponentRegistry::PropertyType::Vector4;

			if (string == "int64_t") return ComponentRegistry::PropertyType::Int64;
			if (string == "uint64_t") return ComponentRegistry::PropertyType::UInt64;

			return ComponentRegistry::PropertyType::Unknown;
		}
	}

	bool ComponentRegistry::Register(const std::string& name, const std::string& definitionData, const RegistrationInfo& guid)
	{
		if (auto it = ComponentGUIDs().find(name); it == ComponentGUIDs().end())
		{
			ComponentGUIDs()[name] = guid;
			ComponentGUIDs()[name].name = name;
			
			ParseDefinition(definitionData, ComponentGUIDs()[name]);
			return true;
		}

		return false;
	}

	const std::string ComponentRegistry::GetNameFromGUID(const WireGUID& aGuid)
	{
		for (const auto& it : ComponentGUIDs())
		{
			if (it.second.guid == aGuid)
			{
				return it.first;
			}
		}

		return "Null";
	}

	const ComponentRegistry::RegistrationInfo& ComponentRegistry::GetRegistryDataFromName(const std::string& aName)
	{
		if (auto it = ComponentGUIDs().find(aName); it != ComponentGUIDs().end())
		{
			return it->second;
		}

		static RegistrationInfo empty;
		return empty;
	}

	const ComponentRegistry::RegistrationInfo& ComponentRegistry::GetRegistryDataFromGUID(const WireGUID& aGuid)
	{
		for (const auto& [name, info] : ComponentGUIDs())
		{
			if (info.guid == aGuid)
			{
				return info;
			}
		}

		static RegistrationInfo empty;
		return empty;
	}

	std::unordered_map<std::string, ComponentRegistry::RegistrationInfo>& ComponentRegistry::ComponentGUIDs()
	{
		static std::unordered_map<std::string, RegistrationInfo> impl;
		return impl;
	}

	void ComponentRegistry::ParseDefinition(const std::string& definitionData, RegistrationInfo& outInfo)
	{
		// Find first {, which is the start of the component
		auto it = definitionData.find('{');

		while (it != std::string::npos)
		{
			it = definitionData.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", it);
			if (it == std::string::npos)
			{
				break;
			}

			auto spaceIt = definitionData.find_first_of(' ', it);
			PropertyType type = Utility::PropertyFromString(definitionData.substr(it, spaceIt - it));

			auto nameEndIt = definitionData.find_first_of(";", spaceIt);
			std::string name = definitionData.substr(spaceIt + 1, nameEndIt - spaceIt - 1);
			name[0] = toupper(name[0]);

			if (type != PropertyType::Unknown)
			{
				auto& property = outInfo.properties.emplace_back();
				property.name = name;
				property.type = type;
			}

			it = definitionData.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", nameEndIt);
		}
	}

	void Serializer::SerializeEntityToFile(EntityId aId, const Registry& aRegistry, const std::filesystem::path& aSceneFolder)
	{
		std::vector<uint8_t> data;
		const std::vector<uint8_t> encodedComponentData = aRegistry.GetEntityComponentDataEncoded(aId);
		const uint32_t componentCount = aRegistry.GetComponentCount(aId);

		data.resize(sizeof(EntityId) + sizeof(uint32_t) + encodedComponentData.size());

		memcpy_s(data.data(), sizeof(EntityId), &aId, sizeof(EntityId));
		memcpy_s(&data[sizeof(EntityId)], sizeof(uint32_t), &componentCount, sizeof(uint32_t));
		memcpy_s(&data[sizeof(EntityId) + sizeof(uint32_t)], encodedComponentData.size(), encodedComponentData.data(), encodedComponentData.size());

		if (!std::filesystem::exists(aSceneFolder))
		{
			std::filesystem::create_directories(aSceneFolder);
		}

		std::filesystem::path finalPath = aSceneFolder / (std::string("Entity_") + std::to_string(aId) + ".ent");

		std::ofstream file(finalPath, std::ios::binary);
		file.write(reinterpret_cast<char*>(data.data()), data.size());
		file.close();
	}

	EntityId Serializer::DeserializeEntityToRegistry(const std::filesystem::path& aPath, Registry& aRegistry)
	{
		std::ifstream file(aPath.string(), std::ios::binary);
		if (!file.is_open())
		{
			return 0;
		}

		std::vector<uint8_t> totalData;
		totalData.resize(file.seekg(0, std::ios::end).tellg());
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		file.close();

		size_t offset = 0;

		const EntityId id = *reinterpret_cast<EntityId*>(&totalData[offset]);
		aRegistry.AddEntity(id);

		offset += sizeof(EntityId);

		const uint32_t componentCount = *reinterpret_cast<uint32_t*>(&totalData[offset]);
		offset += sizeof(uint32_t);

		for (uint32_t i = 0; i < componentCount; i++)
		{
			const uint16_t nameSize = *reinterpret_cast<uint16_t*>(&totalData[offset]);
			offset += sizeof(uint16_t);

			std::string name(reinterpret_cast<char*>(&totalData[offset]), nameSize);

			offset += nameSize;

			const ComponentRegistry::RegistrationInfo& registryData = ComponentRegistry::GetRegistryDataFromName(name);

			std::vector<uint8_t> componentData;
			componentData.resize(registryData.size);
			memcpy_s(componentData.data(), componentData.size(), &totalData[offset], registryData.size);
			offset += registryData.size;

			aRegistry.AddComponent(componentData, registryData.guid, id);
		}

		return id;
	}
}
