#ifndef __batyr_db_field_h__
#define __batyr_db_field_h__

#include <libpq-fe.h>

#include <string>
#include <map>


namespace Batyr
{
namespace Db
{

    struct Field {

        /** column name */
        std::string name;

        /** postgresql type name */
        std::string pgTypeName;

        /** postgresql type oid */
        Oid pgTypeOid;

        /** column is part of the primary key */
        bool isPrimaryKey;
    };

    typedef std::map<std::string, Field> FieldMap;
};
};


#endif // __batyr_db_field_h__

