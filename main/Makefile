CXX="g++"
OFFICE_HOME=/usr/lib/libreoffice
OO_SDK_HOME=$(OFFICE_HOME)/sdk
#CFLAGS=-std=c++17 -I/usr/include/LibreOfficeKit -I$(OO_SDK_HOME)/include -I./idl -I/usr/include/qpdf -Wall -ggdb -O0
CFLAGS=-std=c++17 -I/usr/include/LibreOfficeKit -I$(OO_SDK_HOME)/include -I./idl -I/usr/include/qpdf -Wall -O2
LDFLAGS=-std=c++17 -L$(OFFICE_HOME)/program -L$(OO_SDK_HOME)/lib -llibreofficekitgtk -luno_sal -luno_salhelpergcc3 -ldl -luno_cppu -luno_cppuhelpergcc3 -lreglo -lunoidllo -lxmlreaderlo -lqpdf

all : convert-to-pdf-with-metadata

%.o : %.cc
	$(CXX) $(CFLAGS) -c $< -o $@

idl/com/sun/star/document/XDocumentProperties.hpp :
	mkdir idl && cd idl && $(OO_SDK_HOME)/bin/cppumaker $(OFFICE_HOME)/program/types.rdb $(OFFICE_HOME)/program/types/offapi.rdb -T"com.sun.star.lang.XComponent;com.sun.star.document.XDocumentPropertiesSupplier;com.sun.star.document.XDocumentProperties"

main.o : metadata.h

read-metadata.o : metadata.h idl/com/sun/star/document/XDocumentProperties.hpp

convert-to-pdf-with-metadata : main.o read-metadata.o write-metadata.o
	$(CXX) $^ -o $@ $(LDFLAGS)

.PHONY : clean
clean :
	rm -f *.o
