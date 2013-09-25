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


