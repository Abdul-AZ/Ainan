#include "Environment.h"

namespace Ainan {

	int32_t Environment::FindObjectByID(const UUID& id)
	{
		for (int32_t i = 0; i < Objects.size(); i++)
			if (Objects[i]->ID == id)
				return i;

		return -1;
	}
}
