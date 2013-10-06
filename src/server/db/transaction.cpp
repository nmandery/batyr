#include <sstream>
#include <cstring>

#include "server/db/transaction.h"
#include "server/db/connection.h"


using namespace Batyr::Db;


Transaction::Transaction(Connection * _connection)
    :   logger(Poco::Logger::get("Db::Transaction")),
        connection(_connection),
        rollback(false)
{
    poco_debug(logger, "BEGIN");

    // start transaction
    auto res = PQexec(connection->pgconn, "begin;");
    checkResult(res);
    PQclear(res);
}


Transaction::~Transaction()
{
    auto transactionStatus = PQtransactionStatus(connection->pgconn);
    if (rollback || (transactionStatus == PQTRANS_INERROR) || (transactionStatus == PQTRANS_UNKNOWN)) {
        poco_debug(logger, "ROLLBACK");

       auto res = PQexec(connection->pgconn, "rollback;");
       checkResult(res);
       PQclear(res);
    }
    else {
        poco_debug(logger, "COMMIT");

       auto res = PQexec(connection->pgconn, "commit;");
       checkResult(res);
       PQclear(res);
    }

    // run all exitSqls
    for(auto exitSql: exitSqls) {
        try {

            poco_debug(logger, "Running Exit SQL: " + exitSql);

            auto res = PQexec(connection->pgconn, exitSql.c_str());
            checkResult(res);
            PQclear(res);
        }
        catch(DbError &e) {
            poco_warning(logger, "Transaction Exit SQL failed: "+exitSql);
        }
    }
    exitSqls.clear();
}


void
Transaction::checkResult(PGresult * res)
{
    if (res == nullptr) {
        std::string msg = "query result was null: " + std::string(PQerrorMessage(connection->pgconn));
        poco_error(logger, msg.c_str());
        rollback = true;
        throw DbError(msg);
    }

    auto resStatus = PQresultStatus(res);
    if (resStatus == PGRES_FATAL_ERROR) {
        char * sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        char * msg_primary = PQresultErrorField(res, PG_DIAG_MESSAGE_PRIMARY);

        std::stringstream msgstream;
        msgstream << "query failed: " << msg_primary << " [sqlstate: " << sqlstate << "]";
        poco_error(logger, msgstream.str().c_str());

        // free the resultset
        PQclear(res);

        rollback = true;

        throw DbError(msgstream.str());
    }
}

void
Transaction::createTempTable(const std::string existingTableName, const std::string tempTableName)
{
    std::stringstream querystream;

    poco_debug(logger, "creating table " + tempTableName + " based on " + existingTableName);

    // use a simple select into and not table inheritance
    querystream << "select * into temporary " << tempTableName
                << " from " << existingTableName << " limit 0";
    std::string query = querystream.str();

    // postgresql drops temporary tables when the connections ends. But the temporary
    // tables are not needed anymore when the transaction ends. ensuring that they
    // are dropped immetiately after the end of the transaction
    std::stringstream dropTableStream;
    dropTableStream << "drop table if exists " << tempTableName;
    exitSqls.push_back(dropTableStream.str());

    // create the temporary table
    auto res = PQexec(connection->pgconn, query.c_str());
    checkResult(res);
    PQclear(res);

}

FieldMap
Transaction::getTableFields(const std::string tableName)
{
    FieldMap fieldMap;

    const char *paramValues[1] = { tableName.c_str() };
    int paramLengths[1] = { static_cast<int>(tableName.length()) };
    auto res = PQexecParams(connection->pgconn,
                "select pa.attname, pt.typname, pt.oid, coalesce(is_pk.is_pk, 'N')::text as is_pk"
                " from pg_catalog.pg_attribute pa"
                " join pg_catalog.pg_class pc on pc.oid=pa.attrelid and pa.attnum>0"
                " join pg_catalog.pg_type pt on pt.oid=pa.atttypid"
                " left join ("
                "    select pcs.conrelid as relid, unnest(conkey) as attnum, 'Y'::text as is_pk "
                "        from pg_catalog.pg_constraint pcs"
                "        where pcs.contype = 'p'"
                " ) is_pk on is_pk.relid = pt.oid and is_pk.attnum=pa.attnum"
                " where pc.oid = $1::regclass::oid",
            1, NULL, paramValues, paramLengths, NULL, 1);
    checkResult(res);

    for (int i; i < PQntuples(res); i++) {
        auto attname = PQgetvalue(res, i, 0);
        auto typname = PQgetvalue(res, i, 1);
        auto oid = PQgetvalue(res, i, 2);
        auto isPk = PQgetvalue(res, i, 3);

        auto field = &fieldMap[attname];
        if (!field->name.empty()) {
            PQclear(res);
            throw DbError("Tables with multiple columns with the same name are not supported. Column: "+std::string(attname));
        }

        field->name = attname;
        field->pgTypeName = typname;
        field->pgTypeOid = std::atoi(oid);
        field->isPrimaryKey = (std::strcmp(isPk,"Y") == 0);
    }
    PQclear(res);

    return fieldMap;
}
