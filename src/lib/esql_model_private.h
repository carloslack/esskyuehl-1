#ifndef _ESQL_MODEL_PRIVATE_H
#define _ESQL_MODEL_PRIVATE_H

#include <Esskyuehl.h>

extern EAPI int esql_log_dom;
#define ERR(...) EINA_LOG_DOM_ERR(esql_log_dom, __VA_ARGS__)

typedef struct _Esql_Model_Data Esql_Model_Data;
typedef struct _Esql_Model_Connection_Data Esql_Model_Connection_Data;

enum {
   ESQL_MODEL_PROP_NAME = 0,
   ESQL_MODEL_PROP_VALUE,
   ESQL_MODEL_PROP_TYPE
};

struct _Esql_Model_Connection_Data
{
   const char *addr;
   const char *user;
   const char *password;
};

struct _Esql_Model_Data
{
   Eo *obj;
   Esql *e; /**< Esskyuehl object for connecting to servers */
   Eina_List *properties_list;
   Eina_List *children_list;
   Eina_Value *properties;
   int load_pending;
   Esql_Type backend_type; /**< MySQL, SQLite, Postgress... */
   Esql_Query_Id query_id; /**< current query id */
   Esql_Model_Connection_Data conn;
   const char *database_name;
   const char *table_name;
   Esql_Model_Row_Data row;
   Emodel_Load load;
};

#endif

