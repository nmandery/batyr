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
        public:
            DbError(const std::string & message) 
                    : std::runtime_error(message)
            {
            };
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
    };
};
};

#endif // __batyr_db_connection_h__
