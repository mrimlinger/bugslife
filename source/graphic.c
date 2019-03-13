/*!
 \file graphic.c
 \brief / Module gerant les fonction d'affichage
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 2.0 (rendu 2)
 \date 9 avril 2017
 */

#include <stdio.h>
#include <math.h>
#include <GL/glu.h>

#include "graphic.h"

enum Symboles_graphic {RGB = 3, NB_COUL = 11, NB_COTE = 50};

static float liste_couleur[NB_COUL][RGB] =  {{1. , 0. , 0. }, {0. , 1. , 0. },
											 {0. , 0. , 1. }, {1. , 0.7, 0.8},
											 {0. , 1. , 1. }, {1. , 1. , 0. },
											 {0.7, 0.4, 0.1}, {1. , 0.5, 0. },
											 {0.5, 0.5, 0.5}, {1. , 0. , 1. },
											 {0. , 0. , 0. }};

//*********************************************************************************//

//fonction pour dessiner un cercle
void graphic_dessin_cercle (float x, float y, float rayon, int remplissage,
							int epaisseur, int couleur)

{ 	
	int i = 0;
	
	glLineWidth(epaisseur);
	glColor3fv(liste_couleur[couleur]);

	if (remplissage == GRAPHIC_FILLED)
		glBegin(GL_POLYGON);
	else
		glBegin(GL_LINE_LOOP);
  
	for (i = 0; i < NB_COTE; i++)
    {
		float alpha = i * 2.* M_PI / NB_COTE;
		glVertex2f (x + rayon*cos(alpha), y + rayon*sin(alpha));
    }

	glEnd();
}

//*********************************************************************************//

