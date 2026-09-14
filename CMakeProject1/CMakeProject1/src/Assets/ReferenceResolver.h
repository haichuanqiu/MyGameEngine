#pragma once
#include "ReferenceDescription.h"
class ReferenceResolver {
	public:
	EngineObject* GetItem(ReferenceDescription refDes);
	static ReferenceResolver& Instance()
	{
		static ReferenceResolver instance;

		return instance;
	}

};
