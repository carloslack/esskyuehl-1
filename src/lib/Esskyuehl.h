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

#ifndef ESQL_H
#define ESQL_H

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#define ESQL_DEFAULT_PORT_MYSQL      "3306" /**< Convenience define for default MySQL port */
#define ESQL_DEFAULT_PORT_POSTGRESQL "5432" /**< Convenience define for default PostgreSQL port */

/**
 * @defgroup Esql_Events Esql Events
 * @brief Events that are emitted from the library
 * @{
 */
extern int ESQL_EVENT_ERROR; /**< Event emitted on error, ev object is #Esql */
extern int ESQL_EVENT_CONNECT; /**< Event emitted on connection to db, ev object is #Esql */
extern int ESQL_EVENT_DISCONNECT; /**< Event emitted on disconnection from db, ev object is #Esql */
extern int ESQL_EVENT_RESULT; /**< Event emitted on query completion, ev object is #Esql_Res */
/** @} */
/**
 * @defgroup Esql_Typedefs Esql types
 * @brief These types are used throughout the library
 * @{
 */

/**
 * @typedef Esql
 * Base Esskyuehl object for connecting to servers
 */
typedef struct Esql     Esql;
/**
 * @typedef Esql_Res
 * Esskyuehl result set object for managing query results
 */
typedef struct Esql_Res Esql_Res;
/**
 * @typedef Esql_Row
 * Esskyuehl row object for accessing result rows
 */
typedef struct Esql_Row Esql_Row;

/**
 * @typedef Esql_Query_Id
 * Id to use as a reference for a query
 */
typedef unsigned int Esql_Query_Id;

/**
 * @typedef Esql_Query_Cb
 * Callback to use with a query
 * @see esql_query_callback_set
 */
typedef void (*Esql_Query_Cb)(Esql_Res *, void *);

/**
 * @typedef Esql_Connect_Cb
 * Callback to use with a query
 * @see esql_connect_callback_set
 */
typedef void (*Esql_Connect_Cb)(Esql *, void *);

/**
 * @typedef Esql_Type
 * Convenience enum for determining server backend type
 */
typedef enum
{
   ESQL_TYPE_NONE,
#define ESQL_TYPE_DRIZZLE ESQL_TYPE_MYSQL /**< Drizzle supports the mysql protocol */
   ESQL_TYPE_MYSQL,
   ESQL_TYPE_POSTGRESQL,
   ESQL_TYPE_SQLITE
} Esql_Type;

/** @} */
/* lib */
EAPI int             esql_init(void);
EAPI int             esql_shutdown(void);

/* esql */
EAPI Esql           *esql_new(Esql_Type type);
EAPI Esql           *esql_pool_new(int size, Esql_Type type);
EAPI void           *esql_data_get(const Esql *e);
EAPI void            esql_data_set(Esql *e, void *data);
EAPI Esql_Query_Id   esql_current_query_id_get(const Esql *e);
EAPI const char     *esql_current_query_get(const Esql *e);
EAPI const char     *esql_error_get(const Esql *e);
EAPI Eina_Bool       esql_type_set(Esql *e, Esql_Type type);
EAPI Esql_Type       esql_type_get(const Esql *e);
EAPI void            esql_free(Esql *e);

/* connect */
EAPI Eina_Bool       esql_connect(Esql *e, const char *addr, const char *user, const char *passwd);
EAPI void            esql_disconnect(Esql *e);
EAPI void            esql_connect_callback_set(Esql *e, Esql_Connect_Cb cb, void *data);
EAPI Eina_Bool       esql_database_set(Esql *e, const char *database_name);
EAPI const char     *esql_database_get(const Esql *e);
EAPI void            esql_connect_timeout_set(Esql *e, double timeout);
EAPI double          esql_connect_timeout_get(const Esql *e);
EAPI void            esql_reconnect_set(Esql *e, Eina_Bool enable);
EAPI Eina_Bool       esql_reconnect_get(const Esql *e);

/* query */
EAPI Esql_Query_Id   esql_query(Esql *e, void *data, const char *query);
EAPI Esql_Query_Id   esql_query_args(Esql *e, void *data, const char *fmt, ...);
EAPI Esql_Query_Id   esql_query_vargs(Esql *e, void *data, const char *fmt, va_list args);
EAPI Eina_Bool       esql_query_callback_set(Esql_Query_Id id, Esql_Query_Cb callback);

/* res */
EAPI Esql           *esql_res_esql_get(const Esql_Res *res);
EAPI const char     *esql_res_error_get(const Esql_Res *res);
EAPI void           *esql_res_data_get(const Esql_Res *res);
EAPI Esql_Query_Id   esql_res_query_id_get(const Esql_Res *res);
EAPI const char     *esql_res_query_get(const Esql_Res *res);
EAPI int             esql_res_rows_count(const Esql_Res *res);
EAPI int             esql_res_cols_count(const Esql_Res *res);
EAPI const char     *esql_res_col_name_get(const Esql_Res *res, unsigned int column);

EAPI Esql_Res       *esql_res_ref(Esql_Res *res);
EAPI void            esql_res_unref(Esql_Res *res);
EAPI int             esql_res_refcount(const Esql_Res *res);

EAPI long long int   esql_res_rows_affected(const Esql_Res *res);
EAPI long long int   esql_res_id(const Esql_Res *res);
EAPI Eina_Iterator  *esql_res_row_iterator_new(const Esql_Res *res);

/* convert */
EAPI const char     *esql_res_to_string(const Esql_Res *res);
EAPI unsigned char  *esql_res_to_blob(const Esql_Res *res, unsigned int *size);
EAPI long long int   esql_res_to_lli(const Esql_Res *res);
EAPI double          esql_res_to_double(const Esql_Res *res);
EAPI unsigned long int esql_res_to_ulong(const Esql_Res *res);

/* row */
EAPI const Eina_Value *esql_row_value_struct_get(const Esql_Row *r);
EAPI Eina_Bool         esql_row_value_column_get(const Esql_Row *r, unsigned int column, Eina_Value *val);
EAPI Esql_Res         *esql_row_res_get(const Esql_Row *r);

#endif
