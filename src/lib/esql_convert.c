/*
 * Copyright 2011, 2012 Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "esql_private.h"

/**
 * @defgroup Esql_Convert Conversion
 * @brief Utility functions for converting single cell results
 * @{*/

/**
 * @brief Convert result to a string
 * @param res Result
 * @return Stringshared result
 */
const char *
esql_res_to_string(const Esql_Res *res)
{
   const Esql_Row *row;
   const Esql_Cell *cell;
   Eina_Value tmp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(res, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(res->row_count > 1, NULL);
   if (!res->row_count) return NULL;
   row = EINA_INLIST_CONTAINER_GET(res->rows, Esql_Row);
   cell = EINA_INLIST_CONTAINER_GET(row->cells, Esql_Cell);

   if (!eina_value_setup(&tmp, EINA_VALUE_TYPE_STRINGSHARE))
     return NULL;
   if (!eina_value_convert(&(cell->value), &tmp))
     return NULL;

   return tmp.value.ptr; /* no flush, ptr is a stringshared string! */
}

/**
 * @brief Convert result to a binary blob
 * @param res Result
 * @return Allocated binary blob (must be freed)
 */
unsigned char *
esql_res_to_blob(const Esql_Res *res, unsigned int *size)
{
   const Esql_Row *row;
   const Esql_Cell *cell;
   Eina_Value tmp;
   Eina_Value_Blob blob;
   unsigned char *ret = NULL;

   if (size) *size = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(res, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(res->row_count > 1, NULL);
   if (!res->row_count) return NULL;
   row = EINA_INLIST_CONTAINER_GET(res->rows, Esql_Row);
   cell = EINA_INLIST_CONTAINER_GET(row->cells, Esql_Cell);

   if (!eina_value_setup(&tmp, EINA_VALUE_TYPE_BLOB))
     return NULL;

   if (!eina_value_convert(&(cell->value), &tmp))
     goto error;

   if (!eina_value_pget(&tmp, &blob))
     goto error;

   ret = malloc(blob.size);
   EINA_SAFETY_ON_NULL_GOTO(ret, error);

   memcpy(ret, blob.memory, blob.size);
   if (size) *size = blob.size;

 error:
   eina_value_flush(&tmp);
   return ret;
}

/**
 * @brief Convert result to a long long int
 * @param res Result
 * @return The result
 */
long long int
esql_res_to_lli(const Esql_Res *res)
{
   const Esql_Row *row;
   const Esql_Cell *cell;
   Eina_Value tmp;
   long long int ret = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(res, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(res->row_count > 1, 0);
   if (!res->row_count) return 0;
   row = EINA_INLIST_CONTAINER_GET(res->rows, Esql_Row);
   cell = EINA_INLIST_CONTAINER_GET(row->cells, Esql_Cell);

   if (!eina_value_setup(&tmp, EINA_VALUE_TYPE_INT64))
     return 0;

   if (!eina_value_convert(&(cell->value), &tmp))
     goto error;

   if (!eina_value_pget(&tmp, &ret))
     ret = 0;

 error:
   eina_value_flush(&tmp);
   return ret;
}

/**
 * @brief Convert result to a double
 * @param res Result
 * @return The result
 */
double
esql_res_to_double(const Esql_Res *res)
{
   const Esql_Row *row;
   const Esql_Cell *cell;
   Eina_Value tmp;
   double ret = 0.0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(res, 0.0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(res->row_count > 1, 0.0);
   if (!res->row_count) return 0.0;
   row = EINA_INLIST_CONTAINER_GET(res->rows, Esql_Row);
   cell = EINA_INLIST_CONTAINER_GET(row->cells, Esql_Cell);

   if (!eina_value_setup(&tmp, EINA_VALUE_TYPE_DOUBLE))
     return 0;

   if (!eina_value_convert(&(cell->value), &tmp))
     goto error;

   if (!eina_value_pget(&tmp, &ret))
     ret = 0.0;

 error:
   eina_value_flush(&tmp);
   return ret;
}

/**
 * @brief Convert result to an unsigned long int
 * @param res Result
 * @return Unsigned long int
 */
unsigned long int
esql_res_to_ulong(const Esql_Res *res)
{
   const Esql_Row *row;
   const Esql_Cell *cell;
   Eina_Value tmp;
   unsigned long int ret = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(res, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(res->row_count > 1, 0);
   if (!res->row_count) return 0;
   row = EINA_INLIST_CONTAINER_GET(res->rows, Esql_Row);
   cell = EINA_INLIST_CONTAINER_GET(row->cells, Esql_Cell);

   if (!eina_value_setup(&tmp, EINA_VALUE_TYPE_ULONG))
     return 0;

   if (!eina_value_convert(&(cell->value), &tmp))
     goto error;

   if (!eina_value_pget(&tmp, &ret))
     ret = 0;

 error:
   eina_value_flush(&tmp);
   return ret;
}

/** @} */
