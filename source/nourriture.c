/*!
 \file nouriiture.c
 \brief Module gerant les informations relatives à la nourriture.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nourriture.h"
#include "error.h"
#include "utilitaire.h"
#include "constantes.h"
#include "graphic.h"

enum Symboles_nourriture {NB_PARAM_NOUR = 2, EPAIS = 2};

typedef struct nourriture NOURRITURE;
struct nourriture
{
	int num;
	POINT pos;
	NOURRITURE * precedent;
	NOURRITURE * suivant;
};

static int nb_nourriture = UNDEF;
static NOURRITURE * p_tete = NULL;

//Fonctions locales
static int nourriture_fin_liste (FILE * fichier, int j, char * debut);
static void nourriture_supprime (NOURRITURE * adr);

//*********************************************************************************//

//lecture des informations sur la nourriture
int nourriture_lecture (FILE * fichier)
{
	char mot[MAX_LINE], chaine[MAX_LINE], fin_liste[] = {"FIN_LISTE"};
	char * debut, * fin = NULL, * ligne = NULL;
	int j = 0, k = 0;
	POINT pos_nour;
	
	nb_nourriture = UNDEF;
	
	//recuperation du nombre d'elements de nourriture
	while ((nb_nourriture == UNDEF) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0)
			continue;
		sscanf(chaine, "%d", &nb_nourriture);
	}
	if (utilitaire_EOF(ligne, __func__) == 0) 
		return 0;
	
	//boucle principale de lecture des informations
	while ((j < nb_nourriture) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0)
			continue;
		if (strcmp(mot, fin_liste) == 0)
		{	
			error_lecture_elements_nourriture(ERR_PAS_ASSEZ);
			return 0;
		}
		debut = chaine;
		k = 0;
		while ((k == 0) && (sscanf(debut, "%lf %lf", &(pos_nour.x), 
													 &(pos_nour.y)) 
													 == NB_PARAM_NOUR))
		{	//decalage du pointeur sur la ligne lue
			strtod(debut, &fin);
			debut = fin;
			strtod(debut, &fin);
			debut = fin;
			j++;
			if (j > nb_nourriture-1) k++;
			nourriture_ajout(&pos_nour);
		}
	}
	if (utilitaire_EOF(ligne, __func__) == 0) 
		return 0;
	
	//verification de la presence de FIN_LISTE - elements en trop
	if (nb_nourriture > 0)
	{	
		if (nourriture_fin_liste(fichier, j, debut) == 0)
			return 0;
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée testant FIN_LISTE - elements en trop
static int nourriture_fin_liste (FILE * fichier, int j, char * debut)
{
	char mot[MAX_LINE], chaine[MAX_LINE], fin_liste[] = {"FIN_LISTE"};
	char * ligne = NULL;
	double test_trop = 0;
	int k = 0;
	
	if (sscanf(debut, "%lf", &test_trop) == 1)
	{	
		error_lecture_elements_nourriture(ERR_TROP);
		return 0;
	}
	while ((k == 0) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0)
			continue;
		if (strcmp(mot, fin_liste) != 0)
		{	
			error_lecture_elements_nourriture(ERR_TROP);
			return 0;
		}
		k++;
	}	
	if (utilitaire_EOF(ligne, __func__) == 0) 
		return 0;
	return 1;
}

//*********************************************************************************//

//ajout d'un element de NOURRITURE dans la liste
void nourriture_ajout (POINT * position)
{
	NOURRITURE * nouv = NULL;
	
	if (!(nouv = (NOURRITURE *) malloc(sizeof(NOURRITURE))))
	{
		utilitaire_err_loc(__func__);
		exit (EXIT_FAILURE);
	}
	//chainage de la structure
	nouv -> suivant = p_tete;
	if(p_tete)
		nouv -> suivant -> precedent = nouv;
	p_tete = nouv;
	nouv -> precedent = NULL;
	
	//remplissage des données
	nouv -> pos = * position;
	
}

//*********************************************************************************//

//suppression de l'element NOURRITURE pointe par adr
static void nourriture_supprime (NOURRITURE * adr)
{
		//chainage
		NOURRITURE * apres = adr -> suivant;
		NOURRITURE * avant = adr -> precedent;
		if (avant == NULL)
			p_tete = adr -> suivant;
		else
			adr -> precedent -> suivant = apres;
		if (apres != NULL)
			adr -> suivant -> precedent = avant;
		//liberation de l'espace
		free(adr);
		adr = NULL;
		
}

//*********************************************************************************//

//ecriture des données de la simulation actuelle
void nourriture_ecriture (FILE * sortie)
{
	NOURRITURE * actuel = p_tete;
	int nb_nour = 0;
	
	if (p_tete)
	{
		nb_nour++;
		
		while (actuel -> suivant)
		{
			nb_nour++;
			actuel = actuel -> suivant;
		}
		fprintf(sortie, "%d\n", nb_nour);
		
		while (actuel)
		{	
			fprintf(sortie, "	%lf %lf\n", actuel -> pos.x, actuel -> pos.y);
			actuel = actuel -> precedent;
		}
		fprintf(sortie, "	FIN_LISTE\n");
	}
	else
		fprintf(sortie, "0\n");
		
}

//*********************************************************************************//

//suppression des données de la simulation actuelle
void nourriture_nettoyage (void)
{
	while (p_tete)
	{
		nourriture_supprime(p_tete);
	}
	
	nb_nourriture = UNDEF;
}

//*********************************************************************************//

//affichage de la nourriture dans la fenetre GLUT
void nourriture_affichage (void)
{
	NOURRITURE * actuel = p_tete;
	
	while (actuel)
	{
		graphic_dessin_cercle(actuel -> pos.x, actuel -> pos.y, RAYON_FOOD,
							  GRAPHIC_EMPTY, EPAIS, NOIR);
		actuel = actuel -> suivant;
	}
}

//*********************************************************************************//

//test de superposition avec les éléments de nourriture existants
int nourriture_superposition (POINT pos, double rayon, int suppr)
{
	NOURRITURE * actuel = p_tete;
	double dist = 0, delta = 0;
	
	while (actuel)
	{
		dist = utilitaire_dist_2points(pos, actuel -> pos);
		delta = dist - rayon - RAYON_FOOD;
		
		if (delta < EPSIL_ZERO)
		{
			if (suppr)
				nourriture_supprime(actuel);
			return 0;
		}
		actuel = actuel -> suivant;
	}
	
	return 1;
}

//*********************************************************************************//

//recupération du nombre d'éléments de nourriture
int nourriture_info_nb (void)
{
	NOURRITURE * actuel = p_tete;
	int i = 0;
	
	while (actuel)
	{
		i++;
		actuel = actuel -> suivant;
	}
	
	return i;
}

//*********************************************************************************//

//recupération des positions des éléments de nourriture
void nourriture_info_pos (POINT * tab, int nb_nour)
{
	NOURRITURE * actuel = p_tete;
	int i = 0;
	
	for (i = 0 ; i < nb_nour ; i++)
	{
		tab[i].x = actuel -> pos.x;
		tab[i].y = actuel -> pos.y;
		actuel = actuel -> suivant;
	}
}

//*********************************************************************************//

//suppression des nourritures a partir de leur position
void nourriture_supprime_pos (POINT pos)
{
	NOURRITURE * actuel = p_tete;
	int fait = NON;
	double dist = 0;
	
	while ((actuel) && (fait == NON))
	{
		dist = utilitaire_dist_2points(pos, actuel -> pos);
		if (dist < EPSIL_ZERO)
		{
			nourriture_supprime(actuel);
			fait = OUI;
		}
		actuel = actuel -> suivant;
	}
	
}


