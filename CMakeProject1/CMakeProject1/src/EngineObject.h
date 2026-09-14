#pragma once
#include "Assets/ReferenceDescription.h"
class EngineObject
{
public:
	virtual ~EngineObject() = default;
	ReferenceDescription ReferenceInfo;
};

REFLECT(
	EngineObject,
	FIELD(EngineObject, ReferenceInfo),

)