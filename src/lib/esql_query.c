/*
 * Copyright 2011 Mike Blumenkrantz <mike@zentific.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Esskyuehl.h>
#include <esql_private.h>
#include <stdarg.h>

char *
esql_string_escape(Eina_Bool backslashes, const char *s)
{
   char *ret, *rp;
   const char *p;

   ret = malloc(sizeof(char) * strlen(s) * 2 + 1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   if (!backslashes) /* no backslashes allowed, so just double up single quotes */
     {
        for (p = s, rp = ret; *p; p++, rp++)
          {
             if (*p == '\'')
               {
                  *rp = '\'';
                  rp++;
               }

             *rp = *p;
          }
     }
   else
     {
        for (p = s, rp = ret; *p; p++, rp++)
          {
             char e = 0;
             
             switch (*p)
               {
                case '\'':
                case '\\':
                case '"':
                  e = *p;
                  break;
                case '\n':
                  e = 'n';
                  break;
                case '\r':
                  e = 'r';
                  break;
                case '\0':
                  e = '0';
                  break;
                default:
                  *rp = *p;
                  continue;
               }

             *rp++ = '\\';
             *rp = e;
          }
     }
   *rp = 0;
   return ret;
}

char *
esql_query_escape(Eina_Bool backslashes, size_t *len, const char *fmt, va_list args)
{
   Eina_Strbuf *buf;
   const char *p, *pp;
   char *ret = NULL;

   buf = eina_strbuf_new();
   *len = 0;
   for (p = fmt, pp = strchr(fmt, '%'); p && *p; pp = strchr(p, '%'))
     {
        Eina_Bool l = EINA_FALSE;
        Eina_Bool ll = EINA_FALSE;
        long long int i;
        double d;
        char *s;

        if (!eina_strbuf_append_length(buf, p, pp - p)) goto err;
top:
        switch (pp[1])
          {
           case 0:
             ERR("Invalid format string!");
             goto err;
           case 'l':
             if (!l)
               l = EINA_TRUE;
             else if (!ll)
               ll = EINA_TRUE;
             else
               {
                  ERR("Invalid format string!");
                  goto err;
               }
             pp++;
             goto top;
           case 'f':
             if (l && ll)
               {
                  ERR("Invalid format string!");
                  goto err;
               }
             d = va_arg(args, double);
             if (!eina_strbuf_append_printf(buf, "%lf", d)) goto err;
             break;
           case 'i':
           case 'd':
             if (l && ll)
               i = va_arg(args, long long int);
             else if (l)
               i = va_arg(args, long int);
             else
               i = va_arg(args, int);
             if (!eina_strbuf_append_printf(buf, "%lli", i)) goto err;
             break;
           case 's':
             if (l)
               {
                  ERR("Invalid format string!");
                  goto err;
               }
             s = va_arg(args, char*);
             s = esql_string_escape(backslashes, s);
             if (!s) goto err;
             if (!eina_strbuf_append(buf, s)) goto err;
             free(s);
             break;
           case 'c':
             if (l)
               {
                  ERR("Invalid format string!");
                  goto err;
               }
             {
                char c[3];

                c[0] = va_arg(args, int);
                c[1] = c[2] = 0;
                s = esql_string_escape(backslashes, c);
                if (!s) goto err;
                if (!eina_strbuf_append(buf, s)) goto err;
                free(s);
             }
             break;
           case '%':
             if (!eina_strbuf_append_char(buf, '%')) goto err;
             break;
           default:
             ERR("Unsupported format string: '%s'!", pp);
             goto err;
          }
        
        p = pp + 1;
     }
   *len = eina_strbuf_length_get(buf);
   ret = eina_strbuf_string_steal(buf);
err:
   eina_strbuf_free(buf);
   return ret;
}

/**
 * @defgroup Esql_Query Query
 * @brief Functions to manage/setup queries to databases
 * @{*/
Eina_Bool
esql_query(Esql *e, const char *fmt, ...)
{
   int ret;
   va_list args;
   char *query;

   DBG("(e=%p, fmt='%s')", e, fmt);

   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->backend.db, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fmt, EINA_FALSE);
   if ((!e->fdh) || (!e->connected))
     {
        ERR("Esql object must be connected!");
        return EINA_FALSE;
     }

   va_start(args, fmt);
   query = e->backend.escape(e, fmt, args);
   va_end(args);
   if ((!e->backend_set_funcs) || (e->backend_set_funcs->data == esql_query))
     {
        e->backend.query(e, query);
        ret = e->backend.io(e);
        if (ret == ECORE_FD_ERROR)
          {
             ERR("Connection error: %s", e->backend.error_get(e));
             return EINA_FALSE;
          }
        ecore_main_fd_handler_active_set(e->fdh, ret);
        e->current = ESQL_CONNECT_TYPE_QUERY;
        free(query);
     }
   else
     {
        e->backend_set_funcs = eina_list_append(e->backend_set_funcs, esql_query);
        e->backend_set_params = eina_list_append(e->backend_set_params, query);
     }

   return EINA_TRUE;
}

/** @} */