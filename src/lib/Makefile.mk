pkginclude_HEADERS = src/lib/Esskyuehl.h
pkgincludedir = @includedir@/esskyuehl-@VMAJ@

###
### SQL_MODEL


SUFFIXES = .eo .eo.c .eo.h .eo.legacy.h

%.eo.c: %.eo ${_EOLIAN_GEN_DEP}
	eolian_gen --eo --legacy `pkg-config --variable=eolian_flags eo emodel` --gc -o $@ $<

%.eo.h: %.eo ${_EOLIAN_GEN_DEP}
	eolian_gen --eo `pkg-config --variable=eolian_flags eo emodel` --gh -o $@ $<

%.eo.legacy.h: %.eo ${_EOLIAN_GEN_DEP}
	eolian_gen --legacy `pkg-config --variable=eolian_flags eo emodel` --gh -o $@ $<

CLEANFILES += $(BUILT_SOURCES)

ESQL_EO_GENERATED = \
           src/lib/esql_model.eo.c \
           src/lib/esql_model.eo.h

BUILT_SOURCES = $(ESQL_EO_GENERATED)

esql_modeleolianfilesdir = $(datadir)/eolian/include/esskyuehl-@VMAJ@
esql_modeleolianfiles_DATA = \
              src/lib/esql_model.eo

EXTRA_DIST += $(esql_modeleolianfiles_DATA)


installed_esql_modelmainheadersdir = $(includedir)/esskyuehl-@VMAJ@

dist_installed_esql_modelmainheaders_DATA = src/lib/Esql_Model.h
nodist_installed_esql_modelmainheaders_DATA = $(ESQL_EO_GENERATED)



###


lib_LTLIBRARIES = src/lib/libesskyuehl.la

src_lib_libesskyuehl_la_CFLAGS = \
$(MOD_CFLAGS) \
-DESQL_MODULE_PATH=\"$(libdir)/esskyuehl/$(MODULE_ARCH)\"

src_lib_libesskyuehl_la_LIBADD = \
@EFL_LIBS@

src_lib_libesskyuehl_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

src_lib_libesskyuehl_la_SOURCES = \
src/lib/esql_private.h \
src/lib/esql.c \
src/lib/esql_alloc.c \
src/lib/esql_connect.c \
src/lib/esql_convert.c \
src/lib/esql_events.c \
src/lib/esql_model.c \
src/lib/esql_module.c \
src/lib/esql_module.h \
src/lib/esql_pool.c \
src/lib/esql_query.c \
src/lib/esql_res.c
