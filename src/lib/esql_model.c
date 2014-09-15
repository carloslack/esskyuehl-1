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
   Esql_Type type = ESQL_TYPE_SQLITE;

   EINA_SAFETY_ON_NULL_RETURN(addr);
   EINA_SAFETY_ON_NULL_RETURN(user);
   EINA_SAFETY_ON_NULL_RETURN(password);

   eo_do_super(obj, MY_CLASS, eo_constructor());

   pd->obj = obj;
   pd->e = esql_new(type);
   EINA_SAFETY_ON_NULL_RETURN(pd->e);

   pd->backend_type = type;
   pd->conn.addr = addr;
   pd->conn.user = user;
   pd->conn.password = password;
   pd->load.status = EMODEL_LOAD_STATUS_UNLOADED;

   load.status = EMODEL_LOAD_STATUS_UNLOADED;
   _load_set(pd, load);
}

void _esql_model_eo_base_destructor(Eo *obj, Esql_Model_Data *pd EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN(pd->e);
   esql_free(pd->e);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

Emodel_Load_Status  _esql_model_database_name_set(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char *database_name EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;
   Eina_Bool ret;

   ret = esql_database_set(priv->e, database_name);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EMODEL_LOAD_STATUS_ERROR);
   return pd->load.status;
}

Emodel_Load_Status  _esql_model_database_name_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char **database_name EINA_UNUSED)
{
   Esql_Model_Data *priv = pd;

   *database_name = esql_database_get(priv->e);
   return pd->load.status;
}


Emodel_Load_Status _esql_model_emodel_properties_list_get(Eo *obj EINA_UNUSED,
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

Emodel_Load_Status _esql_model_emodel_property_set(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const char * property EINA_UNUSED, Eina_Value value EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
}

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

   if(esql_isconnected(priv->e) == EINA_FALSE)
     {
        ret = esql_connect(priv->e, priv->conn.addr, priv->conn.user, priv->conn.password);
        EINA_SAFETY_ON_FALSE_RETURN(ret);
     }

   //_esql_model_emodel_properties_load(obj, priv); //XXX: implement
   //_esql_model_emodel_children_load(obj, priv); //XXX: implement


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
   Eina_Bool ret = esql_isconnected(pd->e);

   EINA_SAFETY_ON_FALSE_RETURN(ret);

   esql_disconnect(pd->e);
   load.status = EMODEL_LOAD_STATUS_UNLOADED;
   _load_set(pd, load);
}

Eo * _esql_model_emodel_child_add(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
   return eo_add(ESQL_MODEL_CLASS, obj);
}

Emodel_Load_Status _esql_model_emodel_child_del(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED, Eo *child EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
}

Emodel_Load_Status _esql_model_emodel_children_slice_get(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED,
                                unsigned start EINA_UNUSED, unsigned count EINA_UNUSED, Eina_Accessor **children_accessor EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
}

Emodel_Load_Status _esql_model_emodel_children_count_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, unsigned *children_count EINA_UNUSED)
{
   /**< eina_list_count returns 'unsigned int' */
   *children_count = eina_list_count(pd->children_list);
   return pd->load.status;
}

void _esql_model_emodel_children_load(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
}

#include "esql_model.eo.c"
