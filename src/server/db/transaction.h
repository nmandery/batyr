#ifndef __batyr_db_transaction_h__
#define __batyr_db_transaction_h__

#include <Poco/Logger.h>

#include <libpq-fe.h>

#include <memory>
#include <vector>


namespace Batyr 
{
namespace Db
{
 
    class Connection; // forward decl;

    class Transaction
    {
        private:
            Poco::Logger & logger;
            Connection * connection;

            /**
             * sqls which are executed when the transaction ends.
             * these commands will be send to the db regardless
             * if the transaction failed or was successful
             */
            std::vector< std::string > exitSqls;

            Transaction(Connection *);

            /**
             * discard/rollback all changes made in this transaction
             * regardless if an error occured or not
             */
            bool rollback;

            /**
             * check a PQresult if it was a scuccessfull
             * or throw a meaningful DbError
             *
             * will free the resultset in case of an error
             **/
            void checkResult(PGresult *);

        public:
            friend class Connection;

            ~Transaction();

            void discard()
            {
                rollback = true;
            }

            /**
             * create a temporary table based upon the schema
             * of an existing table;
             */
            void createTempTable(const std::string existingTableName, const std::string tempTableName); 

    };

};
};

#endif // __batyr_db_transaction_h__
