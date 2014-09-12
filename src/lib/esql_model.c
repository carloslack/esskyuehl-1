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

/**
 * 'Implements' section
 */
void _esql_model_eo_base_constructor(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
}

void _esql_model_eo_base_destructor(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
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


void _esql_model_emodel_load(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
}

Emodel_Load_Status _esql_model_emodel_load_status_get(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
   return EMODEL_LOAD_STATUS_ERROR;
}

void _esql_model_emodel_unload(Eo *obj EINA_UNUSED, Esql_Model_Data *pd EINA_UNUSED)
{
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
