#ifndef __batyr_db_transaction_h__
#define __batyr_db_transaction_h__

#include <Poco/Logger.h>

#include <libpq-fe.h>

#include "server/db/field.h"

#include <memory>
#include <vector>
#include <tuple>


namespace Batyr 
{
namespace Db
{
 
    class Connection; // forward decl;

    /** 
     * smartpointer to free a PGresult on scope exit
     */
    typedef std::unique_ptr<PGresult, void (*)(PGresult*) > PGresultPtr;

    typedef std::tuple<int, int, int> VersionTuple;

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
             **/
            void checkResult(PGresultPtr & res);

        public:
            friend class Connection;

            ~Transaction();

            /**
             * execute a sql query
             */
            PGresultPtr exec(const std::string &);
            PGresultPtr execParams(const std::string &_sql, int nParams, const Oid *paramTypes,
                        const char * const *paramValues, const int *paramLengths, const int *paramFormats, int resultFormat);
            PGresultPtr prepare(const std::string &stmtName, const std::string &_sql, int nParams, const Oid *paramTypes);
            PGresultPtr execPrepared(const std::string &stmtName, int nParams, const char * const *paramValues, const int *paramLengths,
                        const int *paramFormats, int resultFormat);


            void discard()
            {
                rollback = true;
            }

            /**
             * create a temporary table based upon the schema
             * of an existing table;
             */
            void createTempTable(const std::string &existingTableSchema, const std::string &existingTableName, const std::string &tempTableName); 

            /**
             *
             */
            FieldMap getTableFields(const std::string &tableSchema, const std::string &tableName);

            /**
             * return the SRID of the given geoemtry coluumn by checking the geometry_columns
             * table.
             *
             * if no srid is found, 0 is returned
             */
            int getGeometryColumnSRID(const std::string &tableSchema, const std::string &tableName, const std::string &columnName);


            /**
             * get the version of the installed postgis extension
             */
            VersionTuple getPostGISVersion();
    };

};
};

#endif // __batyr_db_transaction_h__
