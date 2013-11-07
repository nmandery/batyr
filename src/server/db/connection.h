#ifndef __batyr_db_connection_h__
#define __batyr_db_connection_h__

#include <Poco/Logger.h>

#include <libpq-fe.h>

#include <string>
#include <memory>
#include <stdexcept>

#include "server/configuration.h"
#include "server/db/transaction.h"

namespace Batyr
{
namespace Db
{

//    class Transaction; // forward decl

    class DbError : public std::runtime_error
    {
        private:
            std::string sqlstate;

        public:
            DbError(const std::string & message, std::string _sqlstate = "")
                    :   std::runtime_error(message),
                        sqlstate(_sqlstate)
            {};

            ~DbError() throw()
            {};

            std::string getSqlState()
            {
                return sqlstate;
            };

            /**
             * is sqlstate of class 22 ("DataException")
             * see http://www.postgresql.org/docs/8.4/static/errcodes-appendix.html
             */
            bool isDataException() const
            {
                return !sqlstate.empty() && (sqlstate.compare(0, 2, "22") == 0);
            }
    };


    class Connection {

        private:
            Poco::Logger & logger;
            Batyr::Configuration::Ptr configuration;

            /**
             * the lpbpq connection pointer
             */
            PGconn * pgconn;

            /**
             * internal bookkeeping of connection state
             */
            bool connection_ok;

            /**
             * set the name of the application in postgresql to
             * show in pg_stat_activity
             */
            void setApplicationName();

        public:
            Connection(Batyr::Configuration::Ptr _configuration);
            ~Connection();

            std::unique_ptr<Transaction> getTransaction();

            typedef std::shared_ptr<Connection> Ptr;

            friend class Transaction;

            /**
             * setup or check and attempt to restore the database connection
             */
            bool reconnect(bool restore);

            /**
             * close the current connection
             */
            void close();
    };


};
};

#endif // __batyr_db_connection_h__
