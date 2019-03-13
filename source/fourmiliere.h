/*!
 \file fourmiliere.h
 \brief Module gerant les informations relatives aux fourmilières.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#ifndef FOURMILIERE_H
#define FOURMILIERE_H

#include "utilitaire.h"

//prototype de FOURMILIERE (opaque)
typedef struct fourmiliere FOURMILIERE;

//lecture du nombre de fourmilières
int fourmiliere_nb_lecture (FILE * fichier, int * pt_nb_fourmiliere);

//lecture des informations sur les fourmilières
int fourmiliere_lecture (FILE * fichier, int i);

//verification des superpositions
int fourmiliere_superposition (int mort, POINT ** nour_crea, int * nb_crea);

//ecriture des données de la simulation actuelle
void fourmiliere_ecriture (FILE * sortie);

//suppression des données de la simulation actuelle
void fourmiliere_nettoyage (void);

//transfert des infos pour l'affichage GLUI
int fourmiliere_data_glui (int * tab, int * p_nbF, double * tab_rayon);

//affichage des fourmilieres dans la fenetre GLUT
void fourmiliere_affichage (void);

//mise a jour des fourmilieres
void fourmiliere_update (int nb_nour, POINT * nour_pos, int * nour_cons, 
						 POINT ** nour_crea, int * nb_crea);

//test de superposition d'un element quelconque avec differents éléments
int fourmiliere_superposition_simple (POINT pos, double rayon, int option1, 
									  int option2, int option3);

#endif

