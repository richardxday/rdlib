#ifndef __DATABASE__
#define __DATABASE__

#include "strsup.h"
#include "SQLQuery.h"

class Database {
public:
	Database() {}
	virtual ~Database() {}

	/*--------------------------------------------------------------------------------*/
	/** Open admin connection to database server
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool OpenAdmin(const AString& host) {(void)host; return false;}
	
	/*--------------------------------------------------------------------------------*/
	/** Open connection to database server as user and optionally open database
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Open(const AString& host, const AString& username, const AString& password, const AString& database = "") = 0;

	/*--------------------------------------------------------------------------------*/
	/** Check database connection
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool CheckConnection() {return false;}

	/*--------------------------------------------------------------------------------*/
	/** Return whether database connection is open and valid
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool IsOpen() const = 0;
	
	/*--------------------------------------------------------------------------------*/
	/** Close connection
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void Close() = 0;

	/*--------------------------------------------------------------------------------*/
	/** Return error message
	 */
	/*--------------------------------------------------------------------------------*/
	virtual AString GetErrorMessage(bool full = false) {(void)full; return "";}

	/*--------------------------------------------------------------------------------*/
	/** Clear current error information
	 */
	/*--------------------------------------------------------------------------------*/
	virtual void ClearResult() {}

	/*--------------------------------------------------------------------------------*/
	/** Create user
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool AddUser(const AString& username, const AString& password) = 0;

	/*--------------------------------------------------------------------------------*/
	/** Create database
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool CreateDatabase(const AString& name) = 0;

	/*--------------------------------------------------------------------------------*/
	/** Grant privileges
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool GrantPrivileges(const AString& database, const AString& username) = 0;

	/*--------------------------------------------------------------------------------*/
	/** Sanitize and quote string to avoid SQL Injection vulnerabilities
	 */
	/*--------------------------------------------------------------------------------*/
	virtual AString QuoteString(const AString& str) const {return AString::Formatify("'%s'", str.Escapify().str());}

	/*--------------------------------------------------------------------------------*/
	/** Create date string from datetime
	 */
	/*--------------------------------------------------------------------------------*/
	virtual AString DateString(const ADateTime& dt) const {return dt.DateFormat("'%Y-%M-%D %h:%m:%s.%S'");}
	
	/*--------------------------------------------------------------------------------*/
	/** Run simple SQL database query
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool RunSQL(const AString& sql) = 0;

	/*--------------------------------------------------------------------------------*/
	/** Run complex query returning data
	 */
	/*--------------------------------------------------------------------------------*/
	virtual SQLQuery *RunQuery(const AString& sql) = 0;

	/*--------------------------------------------------------------------------------*/
	/** Check to see if table exists
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool TableExists(const AString& name) {(void)name; return false;}
};

#endif
