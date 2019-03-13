/*!
 \file utilitaire.h
 \brief Module regroupant des fonctions de bas niveau.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#ifndef UTILITAIRE_H
#define UTILITAIRE_H

typedef enum Couleurs {ROUGE, VERT, BLEU, ROSE, CYAN, JAUNE, 
					   BRUN, ORANGE, GRIS, VIOLET, NOIR, NB_COUL} COULEURS;
					   
typedef enum Options {NON, OUI} OPTIONS;

typedef enum Statut {UNDEF = -1, MORT, VIVANT} STATUT;

typedef enum Affichage {TOT_FOURMI, TOT_OUV, TOT_GAR, NB_FOOD, NB_INFOS} AFFICHAGE;

typedef struct Point POINT;
struct Point
{
	double x;
	double y;
};

typedef struct Cercle CERCLE;
struct Cercle
{
	POINT centre;
	double rayon;
};


//determine l'appartenance ou non au domaine [-DMAX;DMAX]
int utilitaire_pos_domaine (double x, double y);

//calcule la distance entre deux points
double utilitaire_dist_2points (POINT pos1, POINT pos2);

//determine si une ligne contient des informations utiles ou non (vide, commentaires)
int utilitaire_debut_ligne (char * mot, char * chaine);

//verifie si l'on est arrivé à la fin du fichier
int utilitaire_EOF (char * ligne, const char * func);

//fonction d'erreur avec localisation utile au debugage
void utilitaire_err_loc (const char * func);

//fonction de création de probabilité [0;1]
double utilitaire_prob (void);

//fonction pour effectuer le déplacement d'une fourmi
POINT utilitaire_mouv (POINT pos, POINT but);

#endif

