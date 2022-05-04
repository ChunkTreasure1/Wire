#include "Serialization.h"

namespace Wire
{
	bool ComponentRegistry::Register(const std::string& name, const GUID& guid)
	{
		if (auto it = ComponentGUIDs().find(name); it == ComponentGUIDs().end())
		{
			ComponentGUIDs()[name] = guid;
			return true;
		}

		return false;
	}

	std::unordered_map<std::string, GUID>& ComponentRegistry::ComponentGUIDs()
	{
		static std::unordered_map<std::string, GUID> impl;
		return impl;
	}
}
