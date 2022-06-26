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

			if (string == "AssetHandle") return ComponentRegistry::PropertyType::AssetHandle;

			return ComponentRegistry::PropertyType::Unknown;
		}

		inline std::string ToLower(const std::string& str)
		{
			std::string newStr(str);
			std::transform(str.begin(), str.end(), newStr.begin(), ::tolower);

			return newStr;
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
		constexpr size_t propertyTextLength = 9;

		auto propertyOffsets = GetMemberOffsetsFromString(definitionData);

		// Find first {, which is the start of the component
		size_t offset = definitionData.find_first_of('{');
		offset = definitionData.find("PROPERTY(", offset);

		while (offset != std::string::npos)
		{
			const size_t propertyBegin = offset;
			const size_t propertyEnd = definitionData.find(")", offset);

			if (propertyBegin == std::string::npos)
			{
				break;
			}

			std::string propertySubStr = definitionData.substr(offset + propertyTextLength, propertyEnd - (propertyBegin + propertyTextLength));

			// Handle property options
			{
				std::string propertyName;
				std::string propertyCategory;
				std::string propertySpecialType;

				bool propertySerializable = true;
				bool propertyVisible = true;

				uint32_t propertyMemOffset = 0;
				PropertyType propertyType;

				propertyName = FindValueInString(propertySubStr, "Name");
				propertyCategory = FindValueInString(propertySubStr, "Category");
				propertySpecialType = Utility::ToLower(FindValueInString(propertySubStr, "SpecialType"));

				{
					std::string value = Utility::ToLower(FindValueInString(propertySubStr, "Serializable"));
					propertySerializable = value == "false" ? false : true;
				}

				{
					std::string value = Utility::ToLower(FindValueInString(propertySubStr, "Visible"));
					propertyVisible = value == "false" ? false : true;
				}

				// Find property name and get it's offset
				{
					const size_t propertyOffset = definitionData.find_first_of(';', propertyBegin);
					std::string propertyString = definitionData.substr(propertyBegin, propertyOffset - propertyBegin);

					const auto [type, name] = FindNameAndTypeFromString(propertyString);
					if (propertyName.empty())
					{
						propertyName = name;
						propertyName[0] = toupper(propertyName[0]);
					}

					propertyMemOffset = (uint32_t)propertyOffsets[name];
					propertyType = type;

				}

				if (propertyType != PropertyType::Unknown)
				{
					auto& propertyValues = outInfo.properties.emplace_back();
					propertyValues.category = propertyCategory;
					propertyValues.name = propertyName;
					propertyValues.offset = propertyMemOffset;
					propertyValues.serializable = propertySerializable;
					propertyValues.visible = propertyVisible;
					propertyValues.type = propertyType;

					if (propertySpecialType == "color" && (propertyType == PropertyType::Vector3 || propertyType == PropertyType::Vector4))
					{
						propertyValues.type = propertyType == PropertyType::Vector3 ? PropertyType::Color3 : PropertyType::Color4;
					}
				}
			}

			offset = definitionData.find("PROPERTY(", propertyEnd);
		}
	}

	std::string ComponentRegistry::FindValueInString(const std::string& srcString, const std::string& key)
	{
		std::string result;
		std::string lowerKey = Utility::ToLower(key);

		if (size_t offset = srcString.find(key); offset != std::string::npos)
		{
			const size_t nextPropertyOffset = srcString.find(",", offset);
			const size_t propertyLength = (nextPropertyOffset != std::string::npos ? nextPropertyOffset : srcString.size()) - offset;

			std::string propertySubStr = srcString.substr(offset, propertyLength);

			const size_t equalSignOffset = propertySubStr.find("=");
			const size_t firstLetter = propertySubStr.find_first_not_of(" =", equalSignOffset);

			result = propertySubStr.substr(firstLetter, propertySubStr.size() - firstLetter);
		}

		return result;
	}

	std::unordered_map<std::string, size_t> ComponentRegistry::GetMemberOffsetsFromString(const std::string& srcString)
	{
		std::unordered_map<std::string, size_t> properties;
		size_t memoryOffset = 0;
		size_t stringOffset = srcString.find_first_of('{');

		while (stringOffset != std::string::npos)
		{
			const size_t propertyOffset = srcString.find_first_of(';', stringOffset);
			std::string propertyString = srcString.substr(stringOffset, propertyOffset - stringOffset);

			const auto [type, name] = FindNameAndTypeFromString(propertyString);

			properties.emplace(name, memoryOffset);

			memoryOffset += GetSizeFromType(type);
			stringOffset = srcString.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", propertyOffset);

			// Check for GUID definition
			{
				size_t nextSemiColon = srcString.find_first_of(';', stringOffset);
				if (nextSemiColon != std::string::npos)
				{
					std::string guidSubString = srcString.substr(stringOffset, nextSemiColon - stringOffset);
					if (guidSubString.find("CREATE_COMPONENT_GUID(") != std::string::npos)
					{
						stringOffset = srcString.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", nextSemiColon);
					}
				}
			}
		}

		return properties;
	}

	std::pair<ComponentRegistry::PropertyType, std::string> ComponentRegistry::FindNameAndTypeFromString(const std::string& srcString)
	{
		bool hasProperty = srcString.find("PROPERTY(") != std::string::npos;

		size_t propDefEnd = srcString.find_first_of(')');
		if (propDefEnd == std::string::npos || !hasProperty)
		{
			propDefEnd = 0;
		}

		PropertyType type;
		std::string name;

		size_t nameOffset = 0;

		{
			const size_t nextLetter = srcString.find_first_not_of(' ', propDefEnd + 1);
			const size_t nextSpace = srcString.find_first_of(' ', nextLetter);

			std::string typeString = srcString.substr(nextLetter, nextSpace - nextLetter);
			type = Utility::PropertyFromString(typeString);

			nameOffset = nextSpace;
		}

		{
			const size_t nextLetter = srcString.find_first_not_of(' ', nameOffset);
			size_t endOfName = srcString.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", nextLetter);
			if (endOfName == std::string::npos)
			{
				endOfName = srcString.length();
			}

			name = srcString.substr(nextLetter, endOfName - nextLetter + 1);
		}

		return { type, name };
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
