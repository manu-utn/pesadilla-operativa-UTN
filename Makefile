all: $(DIRS) $(DIR_BIN)/$(BIN)

$(DIRS): ; $(MKDIR) $@

$(DIR_BIN)/$(BIN): $(OBJ)
  $(info "Enlazamos los objetos ("$(notdir $^)") para crear el ejecutable ($(notdir $@)) en $(dir $@)")
  @$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ): $(DIR_OBJ)/%.o: $(DIR_SRC)/%.c $(DIR_DEP)/%.d | $(DIR_DEP)
	$(info Compilamos el archivo fuente ($(notdir $<)) en objeto en $(dir $@))
  $(info Se modificó el archivo ($?))
  @$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c $(DIR_SRC)/$*.c -o $(DIR_OBJ)/$*.o
  @mv -f $(DIR_DEP)/$*.tmp.d $(DIR_DEP)/$*.d && touch $@ # se ejecuta si no hubo error de compilación

# --------------------------------------------------------------------

c clean:
	$(info "Removiendo ejecutable, objetos y dependencias")
  @-$(RM) $(DIR_BIN)/*.out
  @-$(RM) $(DIR_OBJ)/*.o
  @-$(RM) $(DIR_DEP)/*{.d,.tmp.d}

r run: ; @-$(DIR_BIN)/$(BIN)

w watch:
	$(info "Observando cambios en la aplicación...")
  @while true; do $(MAKE) -q || $(MAKE) --no-print-directory; sleep 1; done

$(DEP):
-include $(wildcard $(DEP))

.PHONY: all c clean r run w watch

-include Makefile.cfg
-include functions.mk
