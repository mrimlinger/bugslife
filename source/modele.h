/*!
 \file modele.h
 \brief Module responsable du bon fonctionnement de la simulation
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#ifndef MODELE_H
#define MODELE_H

//identification du mode de lecture et lancement de la lecture
int modele_lecture (int mode, char * nom_fichier);

//lecture des fichiers en mode Error
int modele_error_rendu1 (FILE *fichier);

//verification des superpositions interdites en mode Verif
int modele_verif_rendu2 (void);

//ecriture des données de la simulation actuelle
void modele_ecriture (char const * nom_fichier);

//suppression des données de la simulation actuelle
int modele_nettoyage (void);

//transfert des infos pour l'affichage GLUI
void modele_data_glui (int * tab, int * p_nbF, double * tab_rayon);

//affichage des elements dans la fenetre GLUT
void modele_affichage (void);

//mise a jour de la simulation
void modele_update (int nour_mode);

//generation (aléatoire ou non) d'un élément de nourriture
void modele_nour_gen (int error, double x, double y);

#endif

