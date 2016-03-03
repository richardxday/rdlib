#ifndef __POSTGRES_DATABASE__
#define __POSTGRES_DATABASE__

#include <libpq-fe.h>

#include "Database.h"

class PostgresDatabase : public Database {
public:
	PostgresDatabase();
	virtual ~PostgresDatabase();

	/*--------------------------------------------------------------------------------*/
	/** Open admin connection to database server
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool OpenAdmin(const AString& host);
	
	/*--------------------------------------------------------------------------------*/
	/** Open connection to database server as user and optionally open database
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Open(const AString& host, const AString& username, const AString& password, const AString& database = "");

	/*--------------------------------------------------------------------------------*/
	/** Check database connection
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool CheckConnection();

	/*--------------------------------------------------------------------------------*/
	/** Check database connection
	 */
	/*--------------------------------------------------------------------------------*/
	static bool CheckConnection(const AString& host);

	/*--------------------------------------------------------------------------------*/
	/** Return whether database connection is open and valid
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool IsOpen() const {return isopen;}
	
	/*--------------------------------------------------------------------------------*/
	/** Close connection
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Close();

	/*--------------------------------------------------------------------------------*/
	/** Return error message
	 */
	/*--------------------------------------------------------------------------------*/
	virtual AString GetErrorMessage(bool full = false);

	/*--------------------------------------------------------------------------------*/
	/** Clear current error information
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void ClearResult();

	/*--------------------------------------------------------------------------------*/
	/** Create user
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool AddUser(const AString& username, const AString& password);

	/*--------------------------------------------------------------------------------*/
	/** Create database
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool CreateDatabase(const AString& name);

	/*--------------------------------------------------------------------------------*/
	/** Grant privileges
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool GrantPrivileges(const AString& database, const AString& username);

	/*--------------------------------------------------------------------------------*/
	/** Sanitize and quote string to avoid SQL Injection vulnerabilities
	 */
	/*--------------------------------------------------------------------------------*/
	virtual AString QuoteString(const AString& str) const;
	
	/*--------------------------------------------------------------------------------*/
	/** Run simple SQL database query
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool RunSQL(const AString& sql);

	/*--------------------------------------------------------------------------------*/
	/** Run complex query returning data
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SQLQuery *RunQuery(const AString& sql);

	/*--------------------------------------------------------------------------------*/
	/** Check to see if table exists
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool TableExists(const AString& name);
	
protected:
	static AString GetErrorMessage(PGconn *conn, bool full = false);

	class PostgresQuery : public SQLQuery {
	public:
		PostgresQuery(PostgresDatabase *_db, const AString& query);
		virtual ~PostgresQuery();

		/*--------------------------------------------------------------------------------*/
		/** Get success and error message
		 */
		/*--------------------------------------------------------------------------------*/
		virtual bool    GetResult()       const {return success;}
		virtual AString GetErrorMessage() const {return PostgresDatabase::GetErrorMessage(conn);}

		/*--------------------------------------------------------------------------------*/
		/** Return row about to be fetch and optionally number of rows in total
		 */
		/*--------------------------------------------------------------------------------*/
		virtual uint_t CurrentRow(uint_t *rows = NULL) const;

		/*--------------------------------------------------------------------------------*/
		/** Fetch a row of results
		 */
		/*--------------------------------------------------------------------------------*/
		virtual bool Fetch(AString& results);

	protected:
		/*--------------------------------------------------------------------------------*/
		/** Clear current error information
		 */
		/*--------------------------------------------------------------------------------*/
		void ClearResult();

	protected:
		PostgresDatabase *db;
		PGconn   *conn;
		PGresult *res;
		uint_t   nfields;
		uint_t   nrows;
		uint_t   row;
		bool     success;
	};
	friend class PostgresQuery;
								   
	PGconn *GetConnection() {return conn;}
		
protected:
	AString  connstr;
	PGconn   *conn;
	PGresult *res;
	bool     isopen;
};

#endif
