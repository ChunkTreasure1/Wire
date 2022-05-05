#include "Serialization.h"

#include <fstream>

namespace Wire
{
	bool ComponentRegistry::Register(const std::string& name, const RegistrationInfo& guid)
	{
		if (auto it = ComponentGUIDs().find(name); it == ComponentGUIDs().end())
		{
			ComponentGUIDs()[name] = guid;
			return true;
		}

		return false;
	}

	const std::string ComponentRegistry::GetNameFromGUID(const GUID& aGuid)
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

	std::unordered_map<std::string, ComponentRegistry::RegistrationInfo>& ComponentRegistry::ComponentGUIDs()
	{
		static std::unordered_map<std::string, RegistrationInfo> impl;
		return impl;
	}
	
	void Serializer::SerializeEntityToFile(EntityId aId, const Registry& aRegistry)
	{
		std::vector<uint8_t> data;
		const std::vector<uint8_t> encodedComponentData = aRegistry.GetEntityComponentDataEncoded(aId);
		const uint32_t componentCount = aRegistry.GetComponentCount(aId);

		data.resize(sizeof(EntityId) + sizeof(uint32_t) + encodedComponentData.size());
		
		memcpy_s(data.data(), sizeof(EntityId), &aId, sizeof(EntityId));
		memcpy_s(&data[sizeof(EntityId)], sizeof(uint32_t), &componentCount, sizeof(uint32_t));
		memcpy_s(&data[sizeof(EntityId) + sizeof(uint32_t)], encodedComponentData.size(), encodedComponentData.data(), encodedComponentData.size());
	
		std::ofstream file("Entity.ent", std::ios::binary);
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
