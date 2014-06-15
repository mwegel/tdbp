############################################################
# Matthias Wegel, Oktober 2013
#
# Makefile
############################################################

# Werte fuer Windows
CPPCOMPILER 		= cl
COMPILER_FLAGS		= /EHsc /Ox /I $(BOOST_DIRECTORY)
COMPILER_ARG		= /link /out:bin/tdbp.exe
BOOST_DIRECTORY		= "C:/Programme/boost/boost_1_54_0"
DELETE			= del /Q
DELETE_RECURSIVE_OPTION	= /S
DOC_GENERATOR		= doxygen

# Werte fuer Linux
#CPPCOMPILER 		= g++
#COMPILER_FLAGS		= -Os -o
#COMPILER_ARG		= bin/tdbp
#DELETE			= rm -f
#DELETE_RECURSIVE_OPTION	= -r
#DOC_GENERATOR		= doxygen


# Generiere Programm und Dokumentation
all : build doc

# Erstelle die ausfuehrbare Datei
build : src/main.cpp include/globaltypes.hpp include/tdbp.hpp include/dcgreader.hpp include/wishtree.hpp
	$(CPPCOMPILER) src/main.cpp $(COMPILER_FLAGS) $(COMPILER_ARG) 

# Generiere die Dokumentation
doc : Doxyfile
	$(DOC_GENERATOR) Doxyfile

# Loesche die produzierten Dateien
clean :
	$(DELETE) *.obj 
	$(DELETE) bin/*.exe
	$(DELETE) $(DELETE_RECURSIVE_OPTION) docu/hmtl
	$(DELETE) $(DELETE_RECURSIVE_OPTION) docu/latex
