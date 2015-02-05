#ifndef __batyr_queryvalue_h__
#define __batyr_queryvalue_h__

#include <string>

#include "common/nullablevalue.h"

namespace Batyr
{

    /**
     * a type for storing postgresql values.
     */
    typedef NullableValue<std::string> QueryValue;

};

#endif // __batyr_db_queryvalue_h__

