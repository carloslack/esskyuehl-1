
#ifndef _ESQL_EMODEL_H
#define _ESQL_EMODEL_H

#include <Eo.h>
#include <Emodel.h>

#ifdef __cplusplus
extern "C" {
#endif

//XXX
typedef struct _Esql_Model_Row_Data Esql_Model_Row_Data;
struct _Esql_Model_Row_Data
{
   const char *row_name;
   const char *type;
};


#include <esql_model.eo.h>

#ifdef __cplusplus
}
#endif
#endif //_ESQL_EMODEL_H
