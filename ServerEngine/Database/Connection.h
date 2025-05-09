/*    ServerEngine/Database/Connection.h    */

#pragma once

#include <sql.h>
#include <sqlext.h>

#include "ServerEngine/Concurrency/Queue.h"

class DbConnection
{
public:
    Bool        Connect(SQLHENV environment, String16View connString);
    void        Clear();

    Bool        Execute(String16View query);
    Bool        Fetch();
    Int64       GetRowCount();
    void        Unbind();

public:
    Bool        BindParameter(SQLUSMALLINT paramNum, SQLSMALLINT valueType, SQLSMALLINT paramType, SQLULEN colSize, SQLPOINTER paramValue, SQLLEN* paramLen);
    Bool        BindColumns(SQLUSMALLINT colNum, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLULEN bufLen, SQLLEN* colLen);

private:
    void        HandleError();

private:
    SQLHDBC     mConnection = SQL_NULL_HANDLE;
    SQLHSTMT    mStatement = SQL_NULL_HANDLE;
};

class DbConnectionPool
{
public:
    DbConnectionPool();
    ~DbConnectionPool();

    Bool                        Connect(Int64 count, String16View connString);
    void                        Clear();

    SharedPtr<DbConnection>     Pop();
    void                        Push(SharedPtr<DbConnection> connection);

private:
    RW_LOCK;
    SQLHENV                             mEnvironment = SQL_NULL_HANDLE;
    LockQueue<SharedPtr<DbConnection>>  mConnections;
};
