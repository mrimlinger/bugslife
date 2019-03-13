/*!
 \file graphic.h
 \brief / Module gerant les fonction d'affichage
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 2.0 (rendu 2)
 \date 9 avril 2017
 */

#ifndef GRAPHIC_H
#define GRAPHIC_H

#define GRAPHIC_EMPTY  0
#define GRAPHIC_FILLED 1


//fonction pour dessiner un cercle
void graphic_dessin_cercle (float x, float y, float rayon, int remplissage,
							int epaisseur, int couleur);

#endif
