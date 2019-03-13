/*!
 \file fourmiliere.h
 \brief Module gerant les informations relatives à la nourriture.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#ifndef NOURRITURE_H
#define NOURRITURE_H

#include "utilitaire.h"

//lecture des informations sur la nourriture
int nourriture_lecture (FILE * fichier);

//ajout d'un element de NOURRITURE dans la liste
void nourriture_ajout (POINT * position);

//ecriture des données de la simulation actuelle
void nourriture_ecriture (FILE * sortie);

//suppression des données de la simulation actuelle
void nourriture_nettoyage (void);

//affichage de la nourriture dans la fenetre GLUT
void nourriture_affichage (void);

//test de superposition avec les éléments de nourriture existants
int nourriture_superposition (POINT pos, double rayon, int suppr);

//recupération du nombre d'éléments de nourriture
int nourriture_info_nb (void);

//recupération des positions des éléments de nourriture
void nourriture_info_pos (POINT * tab, int nb_nour);

//suppression des nourritures a partir de leur position
void nourriture_supprime_pos (POINT pos);

#endif

