/*!
 \file utilitaire.c
 \brief Module regroupant des fonctions de bas niveau.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu 3)
 \date 17 mai 2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "utilitaire.h"
#include "error.h"
#include "constantes.h"


//*********************************************************************************//

//determine l'appartenance ou non au domaine [-DMAX;DMAX]
int utilitaire_pos_domaine (double x, double y)
{	
	if ((abs(x) > DMAX) || (abs(y) > DMAX))
		return 0;
	else
		return 1;
}

//*********************************************************************************//

//calcule la distance entre deux points
double utilitaire_dist_2points (POINT pos1, POINT pos2)
{	
	double dist;
	
	dist = sqrt((pos1.x-pos2.x)*(pos1.x-pos2.x) + (pos1.y-pos2.y)*(pos1.y-pos2.y));
	
	return dist;
}

//*********************************************************************************//

//determine si une ligne contient des informations utiles ou non (vide, commentaires)
int utilitaire_debut_ligne (char * mot, char *chaine)
{	
	char nul[] = {"#"};
	
	sscanf(nul, "%s", mot);
	sscanf(chaine, "%s", mot);
	
	if ((chaine[0] == '\n') || (chaine[0] == '\r') || (mot[0] == '#'))
		return 0;
	else
		return 1;
}

//*********************************************************************************//

//verifie si l'on est arrivé à la fin du fichier
int utilitaire_EOF (char * ligne, const char * func)
{	
	if (ligne == NULL)
	{	
		error_fichier_incomplet();
		return 0;
	}
	else 
		return 1;
}

//*********************************************************************************//

//fonction utile au debugage
void utilitaire_err_loc (const char * func)
{
	printf("erreur: malloc - probleme dans la fonction %s", func);
}

//*********************************************************************************//

//fonction de création de probabilité [0;1]
double utilitaire_prob (void)
{
	return ((double) rand()) / ((double) RAND_MAX);
}

//*********************************************************************************//

//fonction pour effectuer le déplacement d'une fourmi
POINT utilitaire_mouv (POINT pos, POINT but)
{
	POINT new_pos = pos, dir;
	double dist = 0;
	
	dist = utilitaire_dist_2points(pos, but);
	
	if (dist >= BUG_SPEED * DELTA_T)
	{
		//dist != 0
		dir.x = (but.x - pos.x) / dist;
		dir.y = (but.y - pos.y) / dist;

		new_pos.x = pos.x + dir.x * DELTA_T;
		new_pos.y = pos.y + dir.y * DELTA_T;
	}
	else
	{
		new_pos.x = but.x;
		new_pos.y = but.y;
	}
	
	return new_pos;
}

