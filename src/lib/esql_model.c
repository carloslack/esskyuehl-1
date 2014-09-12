#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Emodel.h>
#include <Eina.h>
#include <Ecore.h>
#include "Esql_Model.h"

#include "esql_model_private.h"

#define MY_CLASS ESQL_MODEL_CLASS
#define MY_CLASS_NAME "Esql_Model"

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

Emodel_Load_Status _esql_model_emodel_properties_list_get(Eo *obj EINA_UNUSED,
                                Esql_Model_Data *pd EINA_UNUSED, const Eina_List **properties_list EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
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

   Eina_Bool ret = esql_connect(pd->e, pd->conn.addr, pd->conn.user, pd->conn.password);
   EINA_SAFETY_ON_FALSE_RETURN(ret);
   load.status = EMODEL_LOAD_STATUS_LOADED_PROPERTIES;

   load.status = EMODEL_LOAD_STATUS_LOADED;
   _load_set(pd, load);
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
   return NULL;
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
   return EMODEL_LOAD_STATUS_ERROR;
}

void _esql_model_emodel_children_load(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
}

#include "esql_model.eo.c"
