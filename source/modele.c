/*!
 \file modele.c
 \brief Module responsable du bon fonctionnement de la simulation
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "modele.h"
#include "fourmiliere.h"
#include "fourmi.h"
#include "nourriture.h"
#include "utilitaire.h"
#include "error.h"
#include "constantes.h"


enum Mode_lecture {ERROR, VERIF, GRAPHIC, AUTRE};

static int nb_fourmiliere = 0;

static int modele_fin_liste (FILE * fichier, int i);

//*********************************************************************************//

//identification du mode de lecture et lancement de la lecture
int modele_lecture (int mode, char * nom_fichier)
{
	FILE * fichier;
	
	//lecture du fichier
	if ((fichier = fopen(nom_fichier, "r")) != NULL)
	{	
		switch (mode)
		{
			case ERROR :
				if (modele_error_rendu1(fichier) == 0)
				{
					fclose(fichier);
					fichier = NULL;
					return 0;
				}
				error_success();
				break;
			default :
				if ((modele_error_rendu1(fichier) == 0)
				    || (modele_verif_rendu2() == 0))
				{
					fclose(fichier);
					fichier = NULL;
					return 0;
				}
				error_success();
				break;
		}
	}
	else
	{	
		error_fichier_inexistant();
		return 0;
	}
	fclose(fichier);
	fichier = NULL;
	return 1;
}

//*********************************************************************************//

//lecture des fichiers en mode Error
int modele_error_rendu1 (FILE * fichier)
{
	int i = 0;

	if (fourmiliere_nb_lecture(fichier, &nb_fourmiliere) == 0)
		return 0;
	
	//lecture des informations pour chaque fourmilière
	for (i = 0 ; i < nb_fourmiliere ; i++)
	{	
		if (fourmiliere_lecture(fichier, i) == 0)
			return 0;
	}
	
	//lecture de FIN_LISTE des fourmilières
	if (nb_fourmiliere > 0)
	{	
		if (modele_fin_liste(fichier, i) == 0)
			return 0;
	}
	
	//lecture des informations sur la nourriture
	if (nourriture_lecture(fichier) == 0)
		return 0;
	
	return 1;
}

//*********************************************************************************//

//verification des superpositions interdites en mode Verif
int modele_verif_rendu2 (void)
{
	POINT ** point_null = NULL;
	int * int_null = NULL;
	
	if (!fourmiliere_superposition(NON, point_null, int_null))
		return 0;
	
	return 1;
}

//*********************************************************************************//

//fonction non-exportée testant FIN_LISTE
static int modele_fin_liste (FILE * fichier, int i)
{
	char mot[MAX_LINE], chaine[MAX_LINE], fin_liste[] = {"FIN_LISTE"};
	char * ligne = NULL;
	int k = 0;
	
	while ((k == 0) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0)
			continue;
		if (strcmp(mot, fin_liste) != 0)
		{
			error_lecture_elements_fourmiliere(i, ERR_FOURMILIERE, ERR_TROP);
			return 0;
		}
		k++;
	}
	if (utilitaire_EOF(ligne, __func__) == 0)
		return 0;
		
	return 1;
}

//*********************************************************************************//

//suppression des données de la simulation actuelle
int modele_nettoyage (void)
{
	fourmiliere_nettoyage ();
	nourriture_nettoyage ();
	nb_fourmiliere = 0;
	
	return 1;
}

//*********************************************************************************//

//ecriture des données de la simulation actuelle
void modele_ecriture (char const * nom_fichier)
{
	FILE * sortie;
	
	if ((sortie = fopen(nom_fichier, "w")) != NULL)
	{
		fprintf(sortie, "# infos de la simulation : \n\n\n");
		fprintf(sortie, "# fourmilieres\n");
		fprintf(sortie, "%d\n", nb_fourmiliere);
		
		if (nb_fourmiliere > 0)
			fourmiliere_ecriture(sortie);
		
		fprintf(sortie, "# nourriture\n");
		nourriture_ecriture(sortie);
	}
	
	fclose(sortie);
}

//*********************************************************************************//

//transfert des infos pour l'affichage GLUI
void modele_data_glui (int * tab, int * p_nbF, double * tab_rayon)
{
	int i = 0;
	
	//initialisation des tableaux a 0
	for (i = 0 ; i < MAX_FOURMILIERE * NB_INFOS ; i++)
	{
		tab[i] = 0;
	}
	for (i = 0 ; i < MAX_FOURMILIERE ; i++)
	{
		tab_rayon[i] = 0;
	}
	
	//remplissage des tableau
	fourmiliere_data_glui(tab, p_nbF, tab_rayon);
	
}

//*********************************************************************************//

//affichage des elements dans la fenetre GLUT
void modele_affichage (void)
{
	fourmiliere_affichage();
	nourriture_affichage();
}

//*********************************************************************************//

//mise a jour de la simulation
void modele_update (int nour_mode)
{
	int nb_nour = 0, nb_crea = 0, i = 0, * int_null = NULL;
	POINT * nour_crea = NULL, * point_null = NULL;
	
	//generation de nourriture
	if ((nour_mode) && (utilitaire_prob() < FOOD_RATE))
		modele_nour_gen(NON, NON, NON);
		
	nb_nour = nourriture_info_nb();
	
	if (nb_nour)
	{
		POINT nour_pos[nb_nour];
		int nour_cons[nb_nour];
		for (i = 0 ; i < nb_nour ; i++)
		{
			nour_cons[i] = NON;
		}
		nourriture_info_pos(nour_pos, nb_nour);
		
		//mise a jour des fourmilieres et suppression des nourritures consommées
		fourmiliere_update(nb_nour, nour_pos, nour_cons, &nour_crea, &nb_crea);

		for (i = 0 ; i < nb_nour ; i++)
		{
			if (nour_cons[i])
				nourriture_supprime_pos(nour_pos[i]);
		}
	}
	else
		fourmiliere_update(nb_nour, point_null, int_null, &nour_crea, &nb_crea);
		
	//creation des nourritures tombées au sol
	for (i = 0 ; i < nb_crea ; i++)
	{
		nourriture_ajout(&(nour_crea[i]));
	}
	free(nour_crea);
}

//*********************************************************************************//

//generation (aléatoire ou non) d'un élément de nourriture
void modele_nour_gen (int error, double x, double y)
{
	POINT pos;
	int i = 1;
	
	if (error)
	{
		pos.x = x;
		pos.y = y;
		if ((fourmiliere_superposition_simple(pos, RAYON_FOOD, OUI, OUI, NON) == 0)
			|| (nourriture_superposition(pos, RAYON_FOOD, NON) == 0)
			|| !((x <= DMAX) && (x >= -DMAX))
			|| !((y <= DMAX) && (y >= -DMAX)))
		{
			printf("Can't create food here !\n");
		}
		else
			nourriture_ajout(&pos);
	}
	else
	{
		pos.x = (utilitaire_prob()) * (DMAX+DMAX) - DMAX;
		pos.y = (utilitaire_prob()) * (DMAX+DMAX) - DMAX;
		
		while (((!fourmiliere_superposition_simple(pos, RAYON_FOOD, OUI, OUI, NON))
			   ||(!nourriture_superposition(pos, RAYON_FOOD, NON))) && (i))
		{
			i++;
			pos.x = (utilitaire_prob()) * (DMAX+DMAX) - DMAX;
			pos.y = (utilitaire_prob()) * (DMAX+DMAX) - DMAX;
			if (i >= MAX_RAND)
			{
				i = 0;
				printf("Finding free spot for nourriture took too long!\n");
			}
		}
		if (i)
			nourriture_ajout(&pos);
	}
}

