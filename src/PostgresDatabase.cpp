
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DateTime.h"
#include "PostgresDatabase.h"

#include "pg_types.h"

PostgresDatabase::PostgresDatabase() : conn(NULL),
									   res(NULL),
									   isopen(false)
{
}

PostgresDatabase::~PostgresDatabase()
{
	Close();
}

/*--------------------------------------------------------------------------------*/
/** Open admin connection to database server
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::OpenAdmin(const AString& host)
{
	return Open(host, "postgres", "masterpassword");
}

/*--------------------------------------------------------------------------------*/
/** Open connection to database server as user and optionally open database
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::Open(const AString& host, const AString& username, const AString& password, const AString& database)
{
	bool success = false;

	if (!conn) {
		AString str;

		ClearResult();

		connstr.Delete();
		connstr.printf("postgresql://%s:%s@%s", username.str(), password.str(), host.str());

		if (CheckConnection()) {
			str = connstr;
			if (database.Valid()) str.printf("/%s", database.ToLower().str());
		
			success = (((conn = PQconnectdb(str.str())) != NULL) && (PQstatus(conn) == CONNECTION_OK));
			if (success) {
				if (database.Valid()) debug("Connected to database '%s' on %s\n", database.str(), host.str());			
				isopen = true;
			}
			else {
				if (database.Valid()) debug("Failed to connect to database '%s' on %s: %s\n", database.str(), host.str(), GetErrorMessage().str());
				else				  debug("Failed to connect server %s: %s\n", host.str(), GetErrorMessage().str());

				Close();
			}
		}
		else debug("No connection to server!\n");
	}
	
	return success;
}

/*--------------------------------------------------------------------------------*/
/** Check database connection
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::CheckConnection()
{
	bool success = (PQping(connstr) == PQPING_OK);
	debug("Checking connection...%s\n", success ? "success" : "**failure**");
	return success;
}

/*--------------------------------------------------------------------------------*/
/** Check database connection
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::CheckConnection(const AString& host)
{
	AString connstr;

	connstr.printf("postgresql://%s", host.str());

	bool success = (PQping(connstr) == PQPING_OK);
	debug("Checking connection to %s...%s\n", host.str(), success ? "success" : "**failure**");
	return success;
}

/*--------------------------------------------------------------------------------*/
/** Close connection
 */
/*--------------------------------------------------------------------------------*/
void PostgresDatabase::Close()
{
	ClearResult();
	if (conn) {
		PQfinish(conn);
		conn = NULL;
	}
	connstr.Delete();
	isopen = false;
}

/*--------------------------------------------------------------------------------*/
/** Return error message
 */
/*--------------------------------------------------------------------------------*/
AString PostgresDatabase::GetErrorMessage(PGconn *conn, bool full)
{
	AString msg;
	
	if (conn) {
		msg = AString(PQerrorMessage(conn)).SearchAndReplace("\r", "").SearchAndReplace("\n\n", ", ");
		if (msg.EndsWith(", ")) msg = msg.Left(msg.len() - 2);
		if (!full) msg = msg.Line(0);
	}
	else msg = "No connection!";
	
	return msg;
}

AString PostgresDatabase::GetErrorMessage(bool full)
{
	return GetErrorMessage(conn, full);
}

/*--------------------------------------------------------------------------------*/
/** Clear current error information
 */
/*--------------------------------------------------------------------------------*/
void PostgresDatabase::ClearResult()
{
	if (res) {
		PQclear(res);
		res = NULL;
	}
}

/*--------------------------------------------------------------------------------*/
/** Create user
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::AddUser(const AString& username, const AString& password)
{
	bool success = false;

	if ((success = RunSQL(AString::Formatify("create user %s with password '%s'", username.str(), password.str()))) == false) {
		AString errmsg = GetErrorMessage();
		if (errmsg.PosNoCase("already exists") < 0) {
			debug("Failed to create user '%s': %s\n", username.str(), errmsg.str());
		}
		else success = true;
	}

	return success;
}

/*--------------------------------------------------------------------------------*/
/** Create database
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::CreateDatabase(const AString& name)
{
	bool success = false;

	if ((success = RunSQL(AString::Formatify("create database %s", name.str()))) == false) {
		AString errmsg = GetErrorMessage();
		if (errmsg.PosNoCase("already exists") < 0) {
			debug("Failed to create database '%s': %s\n", name.str(), errmsg.str());
		}
		else success = true;
	}
	
	return success;
}

/*--------------------------------------------------------------------------------*/
/** Grant privileges
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::GrantPrivileges(const AString& database, const AString& username)
{
	bool success = false;

	if ((success = RunSQL(AString::Formatify("grant all privileges on database %s to %s", database.str(), username.str()))) == false) {
		debug("Failed to grant privileges on database: %s\n", GetErrorMessage().str());
	}

	return success;
}

/*--------------------------------------------------------------------------------*/
/** Sanitize and quote string to avoid SQL Injection vulnerabilities
 */
/*--------------------------------------------------------------------------------*/
AString PostgresDatabase::QuoteString(const AString& str) const
{
	return AString::Formatify("E'%s'", str.Escapify().str());
}

/*--------------------------------------------------------------------------------*/
/** Run simple SQL database query
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::RunSQL(const AString& sql)
{
	bool success = false;

	if (conn) {
		ClearResult();
		success = (((res = PQexec(conn, sql.str())) != NULL) && (PQresultStatus(res) == PGRES_COMMAND_OK));
		//debug("Running SQL '%s': %s\n", sql.str(), success ? "** success **" : GetErrorMessage().str());
	}

	return success;
}

/*--------------------------------------------------------------------------------*/
/** Run complex query returning data
 */
/*--------------------------------------------------------------------------------*/
SQLQuery *PostgresDatabase::RunQuery(const AString& sql)
{
	SQLQuery *query = NULL;

	if (conn) {
		ClearResult();
		query = new PostgresQuery(this, sql);
	   
	}
	
	return query;
}

/*--------------------------------------------------------------------------------*/
/** Check to see if table exists
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::TableExists(const AString& name)
{
	SQLQuery *query = NULL;

	if ((query = RunQuery(AString("select count(*) from %").Arg(name))) != NULL) {
		bool success = query->GetResult();
		delete query;
		return success;
	}

	return false;
}

/*----------------------------------------------------------------------------------------------------*/

PostgresDatabase::PostgresQuery::PostgresQuery(PostgresDatabase *_db, const AString& query) : SQLQuery(),
																							  db(_db),
																							  conn(db->GetConnection()),
																							  res(NULL),
																							  nfields(0),
																							  nrows(0),
																							  row(0),
																							  success(false)
{
	if (((res = PQexec(conn, query.str())) != NULL) && (PQresultStatus(res) == PGRES_TUPLES_OK)) {		
		nfields = PQnfields(res);
		nrows   = PQntuples(res);
		//debug("Query '%s' returns %u rows x %u columns\n", query.str(), nrows, nfields);
		success = true;
	}
	//else debug("Query '%s' failed: %s\n", query.str(), GetErrorMessage().str());
}

PostgresDatabase::PostgresQuery::~PostgresQuery()
{
	ClearResult();
}

/*--------------------------------------------------------------------------------*/
/** Clear current error information
 */
/*--------------------------------------------------------------------------------*/
void PostgresDatabase::PostgresQuery::ClearResult()
{
	if (res) {
		PQclear(res);
		res = NULL;
	}
}

/*--------------------------------------------------------------------------------*/
/** Return row about to be fetch and optionally number of rows in total
 */
/*--------------------------------------------------------------------------------*/
uint_t PostgresDatabase::PostgresQuery::CurrentRow(uint_t *rows) const
{
	if (rows) rows[0] = nrows;
	return row;
}

/*--------------------------------------------------------------------------------*/
/** Fetch a row of results
 */
/*--------------------------------------------------------------------------------*/
bool PostgresDatabase::PostgresQuery::Fetch(AString& results)
{
	bool success = false;

	if (res && nfields && (row < nrows)) {
		uint_t i;
		
		results.Delete();

		for (i = 0; i < nfields; i++) {
			if (i) results.printf(",");

			const char *p = PQgetvalue(res, row, i);
			switch (PQftype(res, i)) {
				case TIMESTAMPOID: {
					ADateTime dt;
					dt.FromTimeStamp(p, true);
					results += AString((uint64_t)dt);
					//debug("%s->%llu->%s (%s)\n", p, (uint64)dt, dt.DateFormat("%Y-%M-%D %h:%m:%s.%S").str(), dt.UTCToLocal().DateFormat("%Y-%M-%D %h:%m:%s.%S").str());
					break;
				}
						
				case TEXTOID:
				case CHAROID:
				case VARCHAROID:
					results.printf("'%s'", AString(p).Escapify().str());
					break;

				default:
					results.printf("%s", p);
					break;
			}
		}

		//debug("Row %u/%u: %s\n", row, nrows, results.str());
		row++;
		success = true;
	}
	
	return success;
}
