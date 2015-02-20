#ifndef __batyr_db_connection_h__
#define __batyr_db_connection_h__

#include <Poco/Logger.h>

#include <libpq-fe.h>

#include <string>
#include <memory>
#include <stdexcept>

#include "core/configuration.h"
#include "core/db/transaction.h"

namespace Batyr
{
namespace Db
{

//    class Transaction; // forward decl

    class DbError : public std::runtime_error
    {
        private:
            std::string sqlstate;
            std::string context;

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

            bool hasContext() const
            {
                return !context.empty();
            }

            std::string getContext() const
            {
                return context;
            }

            void setContext(const std::string & c)
            {
                context = c;
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

            /**
             * version of the postgresql server
             * according to the syntax of PQserverVersion
             */
            int getVersion();
    };


};
};

#endif // __batyr_db_connection_h__
