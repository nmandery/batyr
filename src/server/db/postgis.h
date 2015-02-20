#ifndef __batyr_db_postgis_h__
#define __batyr_db_postgis_h__

#include "server/db/transaction.h"


// magic value no srid info in postgis
#define POSTGIS_NO_SRID_FOUND -555

namespace Batyr 
{
namespace Db
{
namespace PostGis 
{

    typedef std::tuple<int, int, int> VersionTuple;
    
    /**
     * get the version of the installed postgis extension
     */
    VersionTuple getVersion(Batyr::Db::Transaction &transaction);


    /**
     * return the SRID the postgis version uses to mark an 
     * undefined SRS
     *
     * This value changed with version 2.0.
     * see https://trac.osgeo.org/postgis/ticket/286
     */
    int getUndefinedSRIDValue(const VersionTuple &versionPostgis);


    /**
     * return the SRID of the given geoemtry coluumn by checking the geometry_columns
     * table.
     *
     * if no srid is found, POSTGIS_NO_SRID_FOUND is returned
     */
    int getGeometryColumnSRID(Batyr::Db::Transaction &transaction, const std::string &tableSchema, 
            const std::string &tableName, const std::string &columnName);

}
}
}

#endif // __batyr_db_postgis_h__
