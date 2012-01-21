/*
 * Copyright 2012 Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
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
#include <sqlite3.h>

static const char *esql_sqlite_error_get(Esql *e);
static void esql_sqlite_disconnect(Esql *e);
static int esql_sqlite_fd_get(Esql *e);
static Ecore_Fd_Handler_Flags esql_sqlite_connect(Esql *e);
static Ecore_Fd_Handler_Flags esql_sqlite_io(Esql *e);
static void esql_sqlite_setup(Esql *e, const char *addr, const char *user, const char *passwd);
static void esql_sqlite_query(Esql *e, const char *query, unsigned int len);
static void esql_sqlite_res_free(Esql_Res *res);
static void esql_sqlite_res(Esql_Res *res);
static char *esql_sqlite_escape(Esql *e, unsigned int *len, const char *fmt, va_list args);
static void esql_sqlite_row_add(Esql_Res *res);
static void esql_sqlite_free(Esql *e);


static const char *
esql_sqlite_error_get(Esql *e)
{
   return sqlite3_errmsg(e->backend.db);
}

static void
esql_sqlite_disconnect(Esql *e)
{
   if (!e->backend.db) return;
   while (sqlite3_close(e->backend.db));
   e->backend.db = NULL;
   free(e->backend.conn_str);
   e->backend.conn_str = NULL;
   e->backend.conn_str_len = 0;
}

static int
esql_sqlite_fd_get(Esql *e __UNUSED__)
{
   /* dummy return */
   return -1;
}

static void
esql_sqlite_thread_end_cb(Esql *e, Ecore_Thread *et __UNUSED__)
{
   e->backend.thread = NULL;
   esql_call_complete(e);
}

static void
esql_sqlite_connect_cancel_cb(Esql *e, Ecore_Thread *et __UNUSED__)
{
   e->backend.thread = NULL;
   esql_event_error(e);
}

static void
esql_sqlite_connect_cb(Esql *e, Ecore_Thread *et)
{
   if (sqlite3_open_v2(e->backend.conn_str, (struct sqlite3 **)&e->backend.db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
     {
        ERR("%s", esql_sqlite_error_get(e));
        ecore_thread_cancel(et);
        e->backend.thread = NULL;
     }
   else
     e->current = ESQL_CONNECT_TYPE_INIT;
}

static Ecore_Fd_Handler_Flags
esql_sqlite_connect(Esql *e)
{
   e->backend.thread = ecore_thread_run((Ecore_Thread_Cb)esql_sqlite_connect_cb,
                                      (Ecore_Thread_Cb)esql_sqlite_thread_end_cb,
                                      (Ecore_Thread_Cb)esql_sqlite_connect_cancel_cb, e);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->backend.thread, ECORE_FD_ERROR);
   return ECORE_FD_READ | ECORE_FD_WRITE;
}

static void
esql_sqlite_query_cb(Esql *e, Ecore_Thread *et)
{
   int tries = 0;
   while (++tries < 1000)
     {
        switch (sqlite3_step(e->backend.stmt))
          {
           case SQLITE_BUSY:
             break;
           case SQLITE_DONE:
             return;
           case SQLITE_ROW:
             if (!e->res)
               {
                  e->res = calloc(1, sizeof(Esql_Res));
                  if (!e->res) goto out;
                  e->res->num_cols = sqlite3_data_count(e->backend.stmt);
                  e->res->affected = sqlite3_changes(e->backend.db);
                  if (!e->res->num_cols) return;
               }
             esql_sqlite_row_add(e->res);
           default:
             goto out;
          }
     }
   /* something crazy is going on */
out:
   ecore_thread_cancel(et);
}

static Ecore_Fd_Handler_Flags
esql_sqlite_io(Esql *e)
{
   e->backend.thread = ecore_thread_run((Ecore_Thread_Cb)esql_sqlite_query_cb,
                                      (Ecore_Thread_Cb)esql_sqlite_thread_end_cb,
                                      (Ecore_Thread_Cb)esql_sqlite_connect_cancel_cb, e);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->backend.thread, ECORE_FD_ERROR);
   return ECORE_FD_READ | ECORE_FD_WRITE;
}

static void
esql_sqlite_setup(Esql *e, const char *addr, const char *user __UNUSED__, const char *passwd __UNUSED__)
{
   e->backend.conn_str = strdup(addr);
}

static void
esql_sqlite_query(Esql *e, const char *query, unsigned int len)
{
   EINA_SAFETY_ON_TRUE_RETURN(sqlite3_prepare_v2(e->backend.db, query, len, (struct sqlite3_stmt**)&e->backend.stmt, NULL));
}

static void
esql_sqlite_res_free(Esql_Res *res)
{
   sqlite3_finalize(res->e->backend.stmt);
   res->e->backend.stmt = NULL;
}

static void
esql_sqlite_res(Esql_Res *res __UNUSED__)
{
}

static char *
esql_sqlite_escape(Esql *e __UNUSED__, unsigned int *len, const char *fmt, va_list args)
{
   return esql_query_escape(EINA_TRUE, len, fmt, args);
}

static void
esql_sqlite_row_add(Esql_Res *res)
{
   Esql_Cell *cell;
   Esql_Row *r;
   int i, cols;

   cols = res->num_cols;

   r = calloc(1, sizeof(Esql_Row));
   EINA_SAFETY_ON_NULL_RETURN(r);
   r->num_cells = res->num_cols;
   r->res = res;
   res->row_count++;
   res->rows = eina_inlist_append(res->rows, EINA_INLIST_GET(r));

   for (i = 0; i < cols; i++)
     {
        cell = calloc(1, sizeof(Esql_Cell));
        EINA_SAFETY_ON_NULL_RETURN(cell);
        cell->row = r;
        cell->colname = sqlite3_column_name(res->e->backend.stmt, i);

        switch (sqlite3_column_type(res->e->backend.stmt, i))
          {
           case SQLITE_TEXT:
             cell->type = ESQL_CELL_TYPE_STRING;
             cell->value.string = (const char*)sqlite3_column_text(res->e->backend.stmt, i);
             cell->len = sqlite3_column_bytes(res->e->backend.stmt, i);
             break;

           case SQLITE_INTEGER:
             cell->type = ESQL_CELL_TYPE_LONG;
             cell->value.i = sqlite3_column_int(res->e->backend.stmt, i);
             break;

           case SQLITE_FLOAT:
             cell->type = ESQL_CELL_TYPE_DOUBLE;
             cell->value.d = sqlite3_column_double(res->e->backend.stmt, i);
             break;

           default:
             cell->type = ESQL_CELL_TYPE_BLOB;
             cell->value.blob = sqlite3_column_blob(res->e->backend.stmt, i);
             cell->len = sqlite3_column_bytes(res->e->backend.stmt, i);
          }
        r->cells = eina_inlist_append(r->cells, EINA_INLIST_GET(cell));
     }
}

static void
esql_sqlite_free(Esql *e)
{
   if (!e->backend.db) return;
   while (sqlite3_close(e->backend.db));
   e->backend.db = NULL;
   e->backend.free = NULL;
   free(e->backend.conn_str);
   e->backend.conn_str = NULL;
   e->backend.conn_str_len = 0;
}

void
esql_sqlite_init(Esql *e)
{
   INFO("Esql type for %p set to SQLite", e);
   e->type = ESQL_TYPE_SQLITE;
   e->backend.connect = esql_sqlite_connect;
   e->backend.disconnect = esql_sqlite_disconnect;
   e->backend.error_get = esql_sqlite_error_get;
   e->backend.setup = esql_sqlite_setup;
   e->backend.io = esql_sqlite_io;
   e->backend.fd_get = esql_sqlite_fd_get;
   e->backend.escape = esql_sqlite_escape;
   e->backend.query = esql_sqlite_query;
   e->backend.res = esql_sqlite_res;
   e->backend.res_free = esql_sqlite_res_free;
   e->backend.free = esql_sqlite_free;
}

Esql_Type
esql_module_init(Esql *e)
{
   if (e) esql_sqlite_init(e);
   return ESQL_TYPE_SQLITE;
}
