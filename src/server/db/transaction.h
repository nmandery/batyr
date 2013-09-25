#ifndef __batyr_db_transaction_h__
#define __batyr_db_transaction_h__

#include <Poco/Logger.h>

#include <memory>


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

            Transaction(Connection *);

            /**
             * discard/rollback all changes made in this transaction
             * regardless if an error occured or not
             */
            bool rollback;

        public:
            friend class Connection;

            ~Transaction();

            void discard()
            {
                rollback = true;
            }
    };

};
};

#endif // __batyr_db_transaction_h__
