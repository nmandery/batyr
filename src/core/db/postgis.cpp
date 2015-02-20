#include <libpq-fe.h>

#include "core/db/postgis.h"
#include "common/stringutils.h"
#include "core/db/connection.h"
#include "common/macros.h"

using namespace Batyr::Db;


PostGis::VersionTuple
PostGis::getVersion(Transaction &transaction)
{
    auto res = transaction.execParams("select postgis_lib_version();",
            0, NULL, NULL, NULL, NULL, 1);
    
    if (PQntuples(res.get()) == 0) {
        throw DbError("postgis_lib_version returned nothing");
    }
    char * versionString = PQgetvalue(res.get(), 0, 0);
    if (versionString == nullptr) {
        throw DbError("postgis_lib_version returned an empty version string");
    }
    auto versionNumbers = StringUtils::split(versionString, '.');
    int versionMajor = 0;
    int versionMinor = 0;
    int versionPatch = 0;

    if (versionNumbers.size()>0) {
        versionMajor = std::atoi(versionNumbers[0].c_str());
    }
    if (versionNumbers.size()>1) {
        versionMinor = std::atoi(versionNumbers[1].c_str());
    }
    if (versionNumbers.size()>2) {
        versionPatch = std::atoi(versionNumbers[2].c_str());
    }

    return VersionTuple(versionMajor, versionMinor, versionPatch);
}


int
PostGis::getUndefinedSRIDValue(const VersionTuple &versionPostgis)
{
    int undefinedSrid = 0;
    if (std::get<0>(versionPostgis) < 2) {
        undefinedSrid = -1;
    }
    return undefinedSrid;
}


int
PostGis::getGeometryColumnSRID(Transaction &transaction, const std::string &tableSchema, 
        const std::string &tableName, const std::string &columnName)
{
    int srid = POSTGIS_NO_SRID_FOUND;
    const char *paramValues[3] = {
            tableSchema.c_str(),
            tableName.c_str(),
            columnName.c_str()
    };
    int paramLengths[COUNT_OF(paramValues)] = {
            static_cast<int>(tableSchema.length()),
            static_cast<int>(tableName.length()),
            static_cast<int>(columnName.length())
    };
    auto res = transaction.execParams(
                "select gc.srid::text"
                " from geometry_columns gc"
                " where gc.f_table_schema = $1::text "
                "   and gc.f_table_name = $2::text"
                "   and gc.f_geometry_column = $3::text",
            COUNT_OF(paramValues), NULL, paramValues, paramLengths, NULL, 1);

    for (int i=0; i < PQntuples(res.get()); i++) {
        char * sridC = PQgetvalue(res.get(), i, 0);
        if (sridC != nullptr) {
            srid = std::atoi(sridC);
        }
    }

    // TODO: Maybe analyze existing geometries for their srid
    
    return srid;
}
