# Compilateur a utiliser :
##########################

CC     = gcc

# Options de compilation : (-g = pour deverminer)
#################################################

CFLAGS = -Wall 


# Librairies a utiliser :
# LIBS: toutes les librairies qu'il faut intégrer
#########################

LIBS   = -lglut -lGL -lGLU -lm -lglui -L/usr/X11R6/lib -lX11 -lXext -lXmu -lXi -lstdc++

# Liste de fichiers source (.c) a compiler :
############################################

CFILES = main.cpp modele.c fourmiliere.c fourmi.c nourriture.c graphic.c error.c utilitaire.c

# Liste de modules objets (generee a partir de CFILES) :
# Les fichiers objets (.o) ont le même nom que les fichiers source (.c)
# Seulement le suffixe change.
########################################################

OFILES = main.o modele.o fourmiliere.o fourmi.o nourriture.o graphic.o error.o utilitaire.o


# edition de liens (rassembler tous les objets -> executable)
#############################################################

rendu3.x: $(OFILES)
	$(CC) $(OFILES) -lm -o rendu3.x $(LIBS)


# compilation separee (production du module objet seulement)

graphic.o: graphic.c
	$(CC) -c graphic.c $(CFLAGS)
main.o: graphic.c modele.o main.cpp 
	$(CC) -c main.cpp $(CFLAGS) 




# Definitions de cibles particulieres :
#
# "make depend" : genere la liste des dependances
# "make clean"  : efface les fichiers .o et .x
#################################################

depend:
	@echo " *** MISE A JOUR DES DEPENDANCES ***"
	@(sed '/^# DO NOT DELETE THIS LINE/q' Makefile && \
	  $(CC) -MM $(CFLAGS) $(CFILES) | \
	  egrep -v "/usr/include" \
	 ) >Makefile.new
	@mv Makefile.new Makefile

clean:
	@echo " *** EFFACE MODULES OBJET ET EXECUTABLE ***"
	@/bin/rm -f *.o rendu3.x

#
# -- Regles de dependances generees par "make depend"
#####################################################
# DO NOT DELETE THIS LINE
main.o: main.cpp modele.h constantes.h tolerance.h
modele.o: modele.c modele.h fourmiliere.h utilitaire.h fourmi.h \
 nourriture.h error.h constantes.h tolerance.h
fourmiliere.o: fourmiliere.c fourmiliere.h utilitaire.h nourriture.h \
 fourmi.h error.h constantes.h tolerance.h graphic.h
fourmi.o: fourmi.c fourmi.h utilitaire.h error.h constantes.h tolerance.h \
 graphic.h
nourriture.o: nourriture.c nourriture.h utilitaire.h error.h constantes.h \
 tolerance.h graphic.h
graphic.o: graphic.c graphic.h
error.o: error.c error.h constantes.h tolerance.h
utilitaire.o: utilitaire.c utilitaire.h error.h constantes.h tolerance.h
