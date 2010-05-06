#include <math.h>
#include "llgfake.h"
#include "spinsystem.h"
#include "spinoperation.h"

LLGFake::LLGFake()
	: LLG("Fake", ENCODE_LLGFAKE)
{
	gamma = 1.0;
}


bool LLGFake::apply(SpinSystem* spinfrom, SpinSystem* fieldfrom, SpinSystem* spinto, bool advancetime)
{
	if(advancetime)
		spinto->time = spinfrom->time + dt;
	return true;
}

