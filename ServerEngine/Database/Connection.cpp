/*    ServerEngine/Database/Connection.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Database/Connection.h"

Bool DbConnection::Connect(SQLHENV environment, String16View connString)
{
    SQLRETURN ret = SQL_SUCCESS;

    ret = ::SQLAllocHandle(SQL_HANDLE_DBC, environment, OUT &mConnection);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        return false;
    }

    Char16 inStr[MAX_PATH] = {};
    ::wcscpy_s(inStr, connString.data());
    Char16 outStr[MAX_PATH] = {};
    SQLSMALLINT outStrLen = 0;

    ret = ::SQLDriverConnect(mConnection, NULL, inStr, NUM_ELEM_16(inStr), OUT outStr, NUM_ELEM_16(outStr), OUT &outStrLen, SQL_DRIVER_NOPROMPT);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        return false;
    }

    ret = ::SQLAllocHandle(SQL_HANDLE_STMT, mConnection, OUT &mStatement);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        return false;
    }

    return true;
}

void DbConnection::Clear()
{
    if (mStatement != SQL_NULL_HANDLE)
    {
        ::SQLFreeHandle(SQL_HANDLE_STMT, mStatement);
        mStatement = SQL_NULL_HANDLE;
    }

    if (mConnection != SQL_NULL_HANDLE)
    {
        ::SQLFreeHandle(SQL_HANDLE_DBC, mConnection);
        mConnection = SQL_NULL_HANDLE;
    }
}

Bool DbConnection::Execute(String16View query)
{
    SQLRETURN ret = ::SQLExecDirect(mStatement, const_cast<SQLWCHAR*>(query.data()), SQL_NTS);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
        return false;
    }

    return true;
}

Bool DbConnection::Fetch()
{
    SQLRETURN ret = ::SQLFetch(mStatement);
    if (ret == SQL_NO_DATA)
    {
        return false;
    }
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
        return false;
    }

    return true;
}

Int64 DbConnection::GetRowCount()
{
    SQLLEN rowCount = 0;
    SQLRETURN ret = ::SQLRowCount(mStatement, OUT &rowCount);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
        return -1;
    }

    return rowCount;
}

void DbConnection::Unbind()
{
    SQLRETURN ret = SQL_SUCCESS;

    ret = ::SQLFreeStmt(mStatement, SQL_UNBIND);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
    }

    ret = ::SQLFreeStmt(mStatement, SQL_CLOSE);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
    }

    ret = ::SQLFreeStmt(mStatement, SQL_RESET_PARAMS);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
    }
}

Bool DbConnection::BindParameter(SQLUSMALLINT paramNum, SQLSMALLINT valueType, SQLSMALLINT paramType, SQLULEN colSize, SQLPOINTER paramValue, SQLLEN* paramLen)
{
    SQLRETURN ret = ::SQLBindParameter(mStatement, paramNum, SQL_PARAM_INPUT, valueType, paramType, colSize, 0, paramValue, 0, OUT paramLen);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
        return false;
    }

    return true;
}

Bool DbConnection::BindColumns(SQLUSMALLINT colNum, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLULEN bufLen, SQLLEN* colLen)
{
    SQLRETURN ret = ::SQLBindCol(mStatement, colNum, targetType, targetValue, bufLen, OUT colLen);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        HandleError();
        return false;
    }

    return true;
}

void DbConnection::HandleError()
{
    SQLSMALLINT recordIdx = 1;
    SQLWCHAR sqlState[MAX_PATH] = {};
    SQLINTEGER nativeError = 0;
    SQLWCHAR errorMsg[MAX_PATH] = {};
    SQLSMALLINT errorMsgLen = 0;

    while (true)
    {
        SQLRETURN ret = ::SQLGetDiagRec(SQL_HANDLE_STMT, mStatement, recordIdx, sqlState, OUT &nativeError, errorMsg, NUM_ELEM_16(errorMsg), OUT &errorMsgLen);
        if ((ret == SQL_NO_DATA) ||
            (SQL_SUCCEEDED(ret) == false))
        {
            break;
        }

        gLogger->Error(errorMsg);
        ++recordIdx;
    }
}

DbConnectionPool::DbConnectionPool()
{}

DbConnectionPool::~DbConnectionPool()
{
    Clear();
}

Bool DbConnectionPool::Connect(Int64 count, String16View connString)
{
    SQLRETURN ret = SQL_SUCCESS;

    WRITE_GUARD;

    ret = ::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, OUT &mEnvironment);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        return false;
    }

    ret = ::SQLSetEnvAttr(mEnvironment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
    if (SQL_SUCCEEDED(ret) == false)
    {
        ASSERT_CRASH(ret != SQL_INVALID_HANDLE, "INVALID_HANDLE");
        return false;
    }

    for (Int64 i = 0; i < count; ++i)
    {
        SharedPtr<DbConnection> connection = std::make_shared<DbConnection>();
        if (connection->Connect(mEnvironment, connString) == false)
        {
            return false;
        }
        mConnections.Push(connection);
    }

    return true;
}

void DbConnectionPool::Clear()
{
    WRITE_GUARD;
    if (mEnvironment != SQL_NULL_HANDLE)
    {
        ::SQLFreeHandle(SQL_HANDLE_ENV, mEnvironment);
        mEnvironment = SQL_NULL_HANDLE;
    }
    mConnections.Clear();
}

SharedPtr<DbConnection> DbConnectionPool::Pop()
{
    SharedPtr<DbConnection> connection;
    mConnections.Pop(connection);

    return connection;
}

void DbConnectionPool::Push(SharedPtr<DbConnection> connection)
{
    mConnections.Push(connection);
}
