#include <cstring>

#include "server/db/connection.h"
#include "server/db/transaction.h"
#include "common/config.h"


using namespace Batyr::Db;


Connection::Connection(Batyr::Configuration::Ptr _configuration)
    :   logger(Poco::Logger::get("Db::Connection")),
        configuration(_configuration),
        pgconn(0),
        connection_ok(true)
{
    poco_debug(logger, "Setting up connection object");

    // set up initial connection
    if (!reconnect(false)) {
        throw DbError("Unable to connect to the database");
    }
}


Connection::~Connection()
{
    close();
}


void
Connection::close()
{
    if (pgconn != nullptr) {
        PQfinish(pgconn);
        pgconn = NULL;
    }
}


bool
Connection::reconnect(bool restore)
{
    if (pgconn != nullptr) {

        // just send a single sql command to ger useful info if
        // the connection is alive
        auto res = PQexec(pgconn, "select 1");
        if (res != nullptr) {
            PQclear(res);
        }

        // check if the connection is fine
        if (PQstatus(pgconn) != CONNECTION_OK) {
            if (restore) {

                // only log this message once per dead connection
                if (connection_ok) {
                    poco_error(logger, "database connection has become bad - trying to reconnect");
                }
                PQreset(pgconn);
                if (PQstatus(pgconn) != CONNECTION_OK) {
                    if (connection_ok) {
                        poco_error(logger, "could not immediately reconnect to database");
                    }
                    connection_ok = false;
                }
                else {
                    connection_ok = true;
                    poco_error(logger, "Successfully reconnected to database");
                    setApplicationName();
                }
            }
            else {
                connection_ok = false;
            }
        }
        else {
            poco_debug(logger, "Existing, open connection is OK");
            connection_ok = true;
        }
    }
    else {
        // establish a new connection
        auto connString = configuration->getDbConnectionString();
        pgconn = PQconnectdb(connString.c_str());

        if (PQstatus(pgconn) != CONNECTION_OK) {
            connection_ok = false;
            poco_error(logger, "Could not connect to the database");
        }
        else {
            connection_ok = true;
            poco_debug(logger, "Successfully connected to the database");
            setApplicationName();
        }
    }
    if (connection_ok) {
        if (PQsetClientEncoding(pgconn, "UTF8") != 0) {
            poco_warning(logger, "Could not set the client_encoding for the database connection");
        }
    }
    return connection_ok;
}


std::unique_ptr<Transaction>
Connection::getTransaction()
{
    std::unique_ptr<Transaction> transaction( new Transaction(this) );
    return std::move(transaction);
}


void
Connection::setApplicationName()
{
    if (pgconn != nullptr) {
        // setting exists only from version 9.0 on
        if (PQserverVersion(pgconn) >= 90000) {

            const char * batyr_name = APP_NAME_SERVER;
            char * batyr_name_e = PQescapeLiteral(pgconn, batyr_name, strlen(batyr_name));
            if (batyr_name_e != nullptr) {

                std::string query("set application_name to " + std::string(batyr_name_e));

                auto res = PQexec(pgconn, query.c_str());
                if (res != nullptr) {
                    PQclear(res);
                }

                PQfreemem(batyr_name_e);
            }
        }
    }
}


