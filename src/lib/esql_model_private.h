#ifndef _ESQL_MODEL_PRIVATE_H
#define _ESQL_MODEL_PRIVATE_H

#include <Esskyuehl.h>

typedef struct _Esql_Model_Data Esql_Model_Data;

struct _Esql_Model_Data
{
   Esql *e; /**< Opaque base Esskyuehl object for connecting to servers */
   Esql_Type backend_type; /**< MySQL, SQLite, Postgress... */
   Esql_Query_Id query_id; /**< current query id */
};

#endif

