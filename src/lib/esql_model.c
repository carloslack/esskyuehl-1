#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Emodel.h>
#include <Eina.h>
#include <Ecore.h>
#include <sqlite3.h> /**< For SQLite datatypes */
#include "Esql_Model.h"

#include "esql_model_private.h"

#define MY_CLASS ESQL_MODEL_CLASS
#define MY_CLASS_NAME "Esql_Model"

static Eina_Value_Struct_Desc *ESQL_MODEL_PROPERTIES_DESC = NULL;
static void _load_set(Esql_Model_Data *pd, Emodel_Load load);

/**
 * SQL query callback
 */
static void EINA_UNUSED
_esql_query_cb(Esql_Res *res EINA_UNUSED, void *data EINA_UNUSED)
{
}

static void EINA_UNUSED
_esql_query_alter_table_cb(Esql_Res *res, void *data EINA_UNUSED)
{
   fprintf(stdout, "Executed query %d: %s\n",esql_res_query_id_get(res), esql_res_query_get(res));
   fprintf(stdout, "Error message: %s\n", esql_res_error_get(res));
   //XXX: Check this, disptach property change notification
}

/**
 * Basically each child will be made of a
 * table column which will then be:
 *
 *    database                   [parent]
 *       table (name + type)     [child]
 *          value                [child's value]
 *
 */
static void EINA_UNUSED
_esql_query_create_table_cb(Esql_Res *res EINA_UNUSED, void *data)
{
   //XXX: This code is still not tested/evaluated so no reason for panic (yet).
   char *query;
   Eina_Bool ret;
   size_t total_len;

   Esql_Model_Data *priv = (Esql_Model_Data *)data;
   size_t table_name_len = strlen(priv->table_name);

   total_len = strlen("ALTER TABLE ") + table_name_len +
       strlen(" ADD COLUMN ") + strlen(priv->row.row_name) + 1 + strlen(priv->row.type);

   query = calloc(1, total_len + 1 /* '\0' terminated string */);
   EINA_SAFETY_ON_NULL_RETURN(query);

   snprintf(query, total_len, "ALTER TABLE %s ADD COLUMN %s %s",
            priv->table_name, priv->row.row_name, priv->row.type);

   Esql_Query_Id id = esql_query(priv->e, data, (const char *)query);
   ret = (id == 0) ? EINA_FALSE : EINA_TRUE;
   EINA_SAFETY_ON_FALSE_GOTO(ret, cleanup);

   fprintf(stdout, "Trying to execute query id %d: %s\n", id, query);

   ret = esql_query_callback_set(id, _esql_query_alter_table_cb);
   EINA_SAFETY_ON_FALSE_GOTO(ret, cleanup);

cleanup:
   free(query);
}

/**
 * Connect callback
 */
static void
_esql_connect_cb(Esql *e EINA_UNUSED, void *data)
{
   Emodel_Load load;
   Esql_Model_Data *priv = (Esql_Model_Data*)data;

   load.status = EMODEL_LOAD_STATUS_LOADED;
   _load_set(priv, load);
}

static void
_load_set(Esql_Model_Data *pd, Emodel_Load load)
{
   if ((pd->load.status & (EMODEL_LOAD_STATUS_LOADED | EMODEL_LOAD_STATUS_LOADING)) &&
                        (load.status & (EMODEL_LOAD_STATUS_LOADED | EMODEL_LOAD_STATUS_LOADING)))
     {
        load.status = pd->load.status | load.status;
        switch (load.status)
          {
           case EMODEL_LOAD_STATUS_LOADED_PROPERTIES:
             load.status &= ~EMODEL_LOAD_STATUS_LOADING_PROPERTIES;
             break;
           case EMODEL_LOAD_STATUS_LOADING_PROPERTIES:
             load.status &= ~EMODEL_LOAD_STATUS_LOADED_PROPERTIES;
             break;
           case EMODEL_LOAD_STATUS_LOADED_CHILDREN:
             load.status &= ~EMODEL_LOAD_STATUS_LOADING_CHILDREN;
             break;
           case EMODEL_LOAD_STATUS_LOADING_CHILDREN:
             load.status &= ~EMODEL_LOAD_STATUS_LOADED_CHILDREN;
             break;
           default: break;
          }
     }

   if (pd->load.status != load.status)
     {
         pd->load.status = load.status;
         eo_do(pd->obj, eo_event_callback_call(EMODEL_EVENT_LOAD_STATUS, &load));
     }
}

static void EINA_UNUSED
_struct_properties_init(void)
{
   typedef struct _This_Esql_Model_Properties
     {
        const char *name;
        unsigned int type;
        Eina_Value value;
     } This_Esql_Model_Properties;

   static Eina_Value_Struct_Member prop_members[] = {
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Esql_Model_Properties, name),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Esql_Model_Properties, value),
     EINA_VALUE_STRUCT_MEMBER(NULL, This_Esql_Model_Properties, type)
   };
   prop_members[ESQL_MODEL_PROP_NAME].type = EINA_VALUE_TYPE_STRING;
   prop_members[ESQL_MODEL_PROP_VALUE].type = EINA_VALUE_TYPE_STRING; /**< XXX: any type from and to string ?! */
   prop_members[ESQL_MODEL_PROP_TYPE].type = EINA_VALUE_TYPE_INT;

   static Eina_Value_Struct_Desc prop_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, // no special operations
     prop_members,
     EINA_C_ARRAY_LENGTH(prop_members),
     sizeof(This_Esql_Model_Properties)
   };
   ESQL_MODEL_PROPERTIES_DESC = &prop_desc;
}

/**
 * Custom constructor
 */
void _esql_model_constructor(Eo *obj, Esql_Model_Data *pd,
                                const char *addr, const char *user, const char *password)
{
   Emodel_Load load;
   Esql_Model_Data *priv = pd;
   Esql_Type type = ESQL_TYPE_SQLITE;

   EINA_SAFETY_ON_NULL_RETURN(addr);
   EINA_SAFETY_ON_NULL_RETURN(user);
   EINA_SAFETY_ON_NULL_RETURN(password);

   eo_do_super(obj, MY_CLASS, eo_constructor());

   priv->obj = obj;
   priv->e = esql_new(type);
   EINA_SAFETY_ON_NULL_RETURN(priv->e);

   priv->backend_type = type;
   priv->conn.addr = addr;
   priv->conn.user = user;
   priv->conn.password = password;
   //EAPI Eina_Bool esql_query_callback_set(Esql_Query_Id id, Esql_Query_Cb callback);

   load.status = EMODEL_LOAD_STATUS_UNLOADED;
   _load_set(priv, load);
}

void _esql_model_eo_base_destructor(Eo *obj, Esql_Model_Data *pd EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;
   EINA_SAFETY_ON_NULL_RETURN(priv->e);
   esql_free(priv->e);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

/**
 * This function should always be called before esql_connect()
 * to ensure portability.
 * @see esql_connect.c
 */
Emodel_Load_Status  _esql_model_database_name_set(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char *database_name EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;
   Eina_Bool ret;

   ret = esql_database_set(priv->e, database_name);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EMODEL_LOAD_STATUS_ERROR);
   priv->database_name = database_name;
   return pd->load.status;
}

Emodel_Load_Status  _esql_model_database_name_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char **database_name EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;

   *database_name = esql_database_get(priv->e);
   return pd->load.status;
}

Emodel_Load_Status
_esql_model_table_add(Eo *obj EINA_UNUSED, Esql_Model_Data *pd, const char *table_name, Esql_Model_Row_Data row)
{
   Esql_Model_Data *priv = pd;
   char *query = NULL;
   size_t tlen;

   EINA_SAFETY_ON_NULL_RETURN_VAL(table_name, EMODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(esql_database_get(priv->e),EMODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(esql_isconnected(priv->e),EMODEL_LOAD_STATUS_ERROR);

   tlen = strlen(table_name);

   query = calloc(1, strlen("CREATE TABLE ") + tlen + 1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(query, EMODEL_LOAD_STATUS_ERROR);

   snprintf(query, tlen, "CREATE TABLE %s", table_name);

   priv->table_name = table_name;
   priv->row.row_name = row.row_name;
   priv->row.type = row.type;

   Esql_Query_Id id = esql_query(priv->e, priv, (const char *)query);
   Eina_Bool ret = esql_query_callback_set(id, _esql_query_create_table_cb);
   if(ret != EINA_TRUE)
     ERR("Error: esql_query_callback_set()\n");

   free(query);

   return pd->load.status;
}

Emodel_Load_Status
_esql_model_emodel_properties_list_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd, const Eina_List **properties_list)
{
   Esql_Model_Data *priv = pd;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv->obj, EINA_FALSE);

   if(priv->properties_list == NULL)
     {
        Eina_Value_Struct_Desc *desc = ESQL_MODEL_PROPERTIES_DESC;
        for(i = 0; i < desc->member_count; ++i)
          priv->properties_list = eina_list_append(priv->properties_list, desc->members[i].name);
     }

   *properties_list = priv->properties_list;

   return priv->load.status;
}

void _esql_model_emodel_properties_load(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
}

/**
 * Execute queries and set callback
 */
Emodel_Load_Status _esql_model_emodel_property_set(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char * property EINA_UNUSED, Eina_Value value EINA_UNUSED)
{
#if 0
   //XXX: check properties schema
   Esql_Model_Data *priv = pd;
   Eina_Value v = value;
   char *query = eina_value_to_string(&v);

   Esql_Query_Id id = esql_query(priv->e, priv, (const char *)query);
   Eina_Bool ret = esql_query_callback_set(id, _esql_query_cb);

   EINA_SAFETY_ON_FALSE_GOTO(ret, error);

   free(query);
   return priv->load.status;
error:
   free(query);
#endif
   return EMODEL_LOAD_STATUS_ERROR;
}

/**
 * Get query values
 */
Emodel_Load_Status _esql_model_emodel_property_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char * property EINA_UNUSED, Eina_Value *value EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
}

void _esql_model_emodel_load(Eo *obj EINA_UNUSED, Esql_Model_Data *pd)
{
   Emodel_Load load;
   Eina_Bool ret;
   Esql_Model_Data *priv = pd;

   EINA_SAFETY_ON_NULL_RETURN(esql_database_get(priv->e));

   if(esql_isconnected(priv->e) == EINA_FALSE)
     {
        /**
         * First we set load status to loading
         */
        load.status = EMODEL_LOAD_STATUS_LOADING;
        _load_set(priv, load);

        /* Set connect callback */
        esql_connect_callback_set(priv->e, _esql_connect_cb, (void*)priv);
        ret = esql_connect(priv->e, priv->conn.addr, priv->conn.user, priv->conn.password);
        EINA_SAFETY_ON_FALSE_RETURN(ret);
        return;
     }

   load.status = EMODEL_LOAD_STATUS_LOADED;
   _load_set(priv, load);
}

Emodel_Load_Status _esql_model_emodel_load_status_get(Eo *obj EINA_UNUSED, Esql_Model_Data *pd)
{
   return pd->load.status;
}

void _esql_model_emodel_unload(Eo *obj EINA_UNUSED, Esql_Model_Data *pd)
{
   Emodel_Load load;
   Esql_Model_Data *priv = pd;
   Eina_Bool ret = esql_isconnected(priv->e);

   EINA_SAFETY_ON_FALSE_RETURN(ret);

   esql_disconnect(priv->e);

   load.status = EMODEL_LOAD_STATUS_UNLOADED;
   _load_set(priv, load);
}

Eo * _esql_model_emodel_child_add(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
   return eo_add(ESQL_MODEL_CLASS, obj);
}

Emodel_Load_Status _esql_model_emodel_child_del(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED, Eo *child EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;

   eo_ref(child);
   return priv->load.status;
}

Emodel_Load_Status _esql_model_emodel_children_slice_get(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED,
                                unsigned start EINA_UNUSED, unsigned count EINA_UNUSED, Eina_Accessor **children_accessor EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
}

Emodel_Load_Status _esql_model_emodel_children_count_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, unsigned *children_count EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;

   /**< eina_list_count returns 'unsigned int' */
   *children_count = eina_list_count(priv->children_list);
   return priv->load.status;
}

void _esql_model_emodel_children_load(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
}

#include "esql_model.eo.c"
