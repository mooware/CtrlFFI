include $(API_ROOT)/CtrlExt.mk

LIBFFI_PATH = libffi/install/lib/libffi-3.0.13
LIBFFI_INCL = $(LIBFFI_PATH)/include
LIBFFI_LIB = $(LIBFFI_PATH)/../libffi.a

INCLUDE += -I$(LIBFFI_INCL)
OFILES += FFIExternHdl.o FFIValue.o
LIBS += $(LIBFFI_LIB)

CtrlFFI: $(OFILES) $(LIBFFI_LIB)
	$(SHLIB) -o CtrlFFI.so $(OFILES) $(LIBS)

$(LIBFFI_LIB) $(LIBFFI_INCL):
	@cd libffi ; ./configure --with-pic --prefix=$(PWD)/libffi/install && make && make install

$(OFILES): $(LIBFFI_INCL)

clean:
	@rm -f *.o CtrlFFI.so
