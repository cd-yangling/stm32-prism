.PHONY: vimdb
vimdb:
	$(Q)find $(CURDIR) $(VIMDB_DIRS) -name '*.[ch]' > cscope.files
	$(Q)rm -f tags cscope.out cscope.out.in cscope.out.po
	$(Q)ctags -R --languages=c++ --langmap=c++:+.c --c++-kinds=+p \
		--fields=+iaS --extras=+q -f tags $(CURDIR) $(VIMDB_DIRS)
	$(Q)cscope -b -q -i cscope.files -f cscope.out
	@echo "  VIMDB   $$(wc -l < cscope.files) files -> tags + cscope.out"

.PHONY: cc
cc:
	$(Q){ echo '['; sep=''; for f in $(C_SOURCES); do \
		printf '%s  {\n    "directory": "%s",\n    "command": "%s %s -c %s",\n    "file": "%s"\n  }' \
			"$$sep" "$(CURDIR)" "$(CC) $(CFLAGS)" "$$f" "$$f"; sep=','; echo; \
	done; echo ']'; } > compile_commands.json
	@echo "  CCDB    compile_commands.json ($$(grep -c '"file"' compile_commands.json) entries)"

.PHONY: distclean
distclean: clean
	$(Q)rm -f tags cscope.files cscope.out cscope.out.in cscope.out.po compile_commands.json
