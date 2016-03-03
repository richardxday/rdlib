#ifndef __SQL_QUERY__
#define __SQL_QUERY__

#include "strsup.h"

/*--------------------------------------------------------------------------------*/
/** SQL query results interface
 */
/*--------------------------------------------------------------------------------*/
class SQLQuery {
public:
	SQLQuery() {}
	virtual ~SQLQuery() {}

	/*--------------------------------------------------------------------------------*/
	/** Get success and error message
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool    GetResult() const = 0;
	virtual AString GetErrorMessage() const = 0;

	/*--------------------------------------------------------------------------------*/
	/** Return row about to be fetch and optionally number of rows in total
	 */
	/*--------------------------------------------------------------------------------*/
	virtual uint_t CurrentRow(uint_t *rows = NULL) const = 0;

	/*--------------------------------------------------------------------------------*/
	/** Fetch a row of results
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool Fetch(AString& results) = 0;
};

#endif

