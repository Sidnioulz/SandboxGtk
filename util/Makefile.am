# The extract_strings tool is a build utility that runs on the build system.
extract_strings_sources = extract-strings.c
extract_strings_cppflags =
extract_strings_cflags = $(GLIB_CFLAGS_FOR_BUILD)
extract_strings_ldadd = $(GLIB_LIBS_FOR_BUILD)

extract-strings: $(extract_strings_sources)
	@rm -f extract-strings
	$(AM_V_CCLD)$(CC_FOR_BUILD) $(extract_strings_cppflags) $(CPPFLAGS_FOR_BUILD) $(extract_strings_cflags) $(CFLAGS_FOR_BUILD) $^ $(LDFLAGS_FOR_BUILD) $(extract_strings_ldadd) $(LIBS_FOR_BUILD) -o $@

noinst_PROGRAMS = extract-strings
