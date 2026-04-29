#include "prpch.h"
#include "Ref.h"

namespace Prism
{

	void PRISM_API Destroy(RefCounted* refCounted)
	{
		delete refCounted;
	}

}