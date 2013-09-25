#include <sstream>

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
    //::execute("begin;");
}


Transaction::~Transaction()
{
    auto transactionStatus = PQtransactionStatus(connection->pgconn);
    if (rollback || (transactionStatus == PQTRANS_INERROR) || (transactionStatus == PQTRANS_UNKNOWN)) {
        poco_debug(logger, "ROLLBACK");
        //::execute("rollback;");
    }
    else {
        poco_debug(logger, "COMMIT");
        //::execute("commit;");
    }
}

 
void 
Transaction::checkResult(const PGresult * res)
{
    if (res == nullptr) {
        std::string msg = "query result was null: " + std::string(PQerrorMessage(connection->pgconn));
        poco_error(logger, msg.c_str()); 
        throw DbError(msg);
    }

    auto resStatus = PQresultStatus(res);
    if (resStatus == PGRES_FATAL_ERROR) {
        char * sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
        char * msg_primary = PQresultErrorField(res, PG_DIAG_MESSAGE_PRIMARY);

        std::stringstream msgstream;
        msgstream << "query failed: " << msg_primary << " [sqlstate: " << sqlstate << "]";
        poco_error(logger, msgstream.str().c_str());
        throw DbError(msgstream.str());
    }
}
