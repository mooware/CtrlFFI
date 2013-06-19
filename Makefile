include $(API_ROOT)/CtrlExt.mk

OFILES = FFIExternHdl.o FFIValue.o

CtrlFFI: $(OFILES)
	@rm -f addVerInfo.o
	@$(MAKE) addVerInfo.o
	$(SHLIB) -o CtrlFFI.so addVerInfo.o $(OFILES) $(LIBS)

clean:
	@rm -f *.o CtrlFFI.so

addVerInfo.cxx: $(API_ROOT)/addVerInfo.cxx
	@cp -f $(API_ROOT)/addVerInfo.cxx addVerInfo.cxx

addVerInfo.o: $(OFILES) addVerInfo.cxx
