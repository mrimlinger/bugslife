/*!
 \file fourmiliere.c
 \brief Module gerant les informations relatives aux fourmilières.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fourmiliere.h"
#include "nourriture.h"
#include "fourmi.h"
#include "utilitaire.h"
#include "error.h"
#include "constantes.h"
#include "graphic.h"

enum Symboles_fourmiliere {NON_LU = -1, EPAIS = 2, NB_PARAM_FOURMILIERE = 6};
enum Type_fourmi {GAR, OUV};

typedef struct fourmiliere FOURMILIERE;
struct fourmiliere 
{
	POINT pos;
	int nbO;
	int nbG;
	double total_food;
	double rayon;
	
	int vie;
	int num;
	int raid;
	
	FOURMILIERE * precedent;
	FOURMILIERE * suivant;
	
	OUVRIERE * ouv_p_tete;
	GARDE * gar_p_tete;
};

static int nb_fourmiliere = NON_LU;

static FOURMILIERE * p_tete;

static void fourmiliere_ajout (FOURMILIERE * fourmiliere_n);
static void fourmiliere_info (int * tab_ouv_nb, POINT ** tab_ouv_pos, 
							  double * fourmiliere_rayon, POINT * fourmiliere_pos,
							  int * fourmiliere_vie, double ** fourmiliere_nour);
static void fourmiliere_naissance (FOURMILIERE * actuel);
static void fourmiliere_nour_update (FOURMILIERE * actuel);
static void fourmiliere_rayon_update (FOURMILIERE * actuel, POINT * fourmiliere_pos,
									  double * fourmiliere_rayon, 
									  int * fourmiliere_vie, int num);
static void fourmiliere_fin (FOURMILIERE ** fml);

//*********************************************************************************//

//lecture du nombre de fourmilières
int fourmiliere_nb_lecture (FILE * fichier, int * pt_nb_fourmiliere)
{
	char mot[MAX_LINE], chaine[MAX_LINE];
	char * ligne = NULL;
	
	nb_fourmiliere = NON_LU;
	
	while ((nb_fourmiliere == NON_LU) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0)
			continue;
		
		if (sscanf(chaine, "%d", &nb_fourmiliere) == 1)
		{	
			if (nb_fourmiliere > MAX_FOURMILIERE)
			{	
				error_nb_fourmiliere(nb_fourmiliere);
				return 0;
			}
		}
	}
	if (utilitaire_EOF(ligne, __func__) == 0)
		return 0;
	* pt_nb_fourmiliere = nb_fourmiliere;

	
	return 1;
}

//*********************************************************************************//

//lecture des informations sur les fourmilières
int fourmiliere_lecture (FILE * fichier, int i)
{	
	FOURMILIERE fourmiliere_n;
	char mot[MAX_LINE], chaine[MAX_LINE], * ligne = NULL;
	double rayon_max;
	int k = 0;
	fourmiliere_n.ouv_p_tete = NULL;
	fourmiliere_n.gar_p_tete = NULL;
	
	fourmiliere_n.num = i;
	fourmiliere_n.vie = VIVANT;
	while ((!k) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0)
			continue;
		
		if (sscanf(chaine, "%lf %lf %d %d %lf %lf", &(fourmiliere_n.pos.x),
													&(fourmiliere_n.pos.y),
													&(fourmiliere_n.nbO),
													&(fourmiliere_n.nbG),
													&(fourmiliere_n.total_food),
													&(fourmiliere_n.rayon)) 
													== NB_PARAM_FOURMILIERE)
		{	
			if (utilitaire_pos_domaine(fourmiliere_n.pos.x,fourmiliere_n.pos.y) == 0)
			{	
				error_pos_domaine(ERR_FOURMILIERE, i, fourmiliere_n.pos.x, 
													  fourmiliere_n.pos.y);
				return 0;
			}
			rayon_max = (1 + sqrt(fourmiliere_n.nbO + fourmiliere_n.nbG) +
						sqrt(fourmiliere_n.total_food)) * RAYON_FOURMI + EPSIL_ZERO;
			if (fourmiliere_n.rayon > rayon_max)
			{	
				error_rayon_fourmiliere(i);
				return 0;
			}
			k++;
			fourmiliere_ajout(&fourmiliere_n);
		}
		else
		{
			error_lecture_elements_fourmiliere(i, ERR_FOURMILIERE, ERR_PAS_ASSEZ);
			return 0;
		}
	}
	if (utilitaire_EOF(ligne, __func__) == 0) 
		return 0;
	//lecture des ouvrieres et des gardes
	if (fourmi_lecture_ouv(fichier, i, p_tete -> nbO, &(p_tete -> ouv_p_tete)) == 0)
		return 0;
	if (fourmi_lecture_gar(fichier, i, p_tete -> nbG, p_tete -> pos, p_tete -> rayon, 
						   &(p_tete -> gar_p_tete)) == 0)
		return 0;
	return 1;
}

//*********************************************************************************//

//verifications des superpositions 				(mort = 0 : message d'erreur)
//												(mort = 1 : mort des fourmis)
int fourmiliere_superposition (int mort, POINT ** nour_crea, int * nb_crea)
{
	FOURMILIERE * act = p_tete, * cmp = NULL, * dernier = NULL;
	double dist = 0;
	int i = 0, k = 0;
	
	fourmiliere_fin(&(act));
	cmp = act;
	dernier = act;
	
	while (act)
	{
		while (cmp)
		{
			if ((k != i) && (act -> vie) && (cmp -> vie))
			{
				dist = utilitaire_dist_2points(act -> pos, cmp -> pos);
				if ((!mort) && (dist <= (act -> rayon) + (cmp -> rayon)))
				{
					error_superposition_fourmiliere(i, k);
					return 0;
				}
				if ((act -> ouv_p_tete) && (cmp -> ouv_p_tete) &&
					(!fourmi_super_OO(&(act -> ouv_p_tete), &(cmp -> ouv_p_tete),
									  &(act -> nbO), &(cmp -> nbO), mort, i, k, 
									  nour_crea, nb_crea)))
					if (!mort)
						return 0;
				if ((act -> ouv_p_tete) && (cmp -> gar_p_tete) && 
					(!fourmi_super_OG(&(act -> ouv_p_tete), &(cmp -> gar_p_tete), 
									  &(act -> nbO), &(cmp -> nbG), mort, i, k, 
									  nour_crea, nb_crea)))
					if (!mort)
						return 0;
				if ((act -> gar_p_tete) && (cmp -> ouv_p_tete) && 
					(!fourmi_super_OG(&(cmp -> ouv_p_tete), &(act -> gar_p_tete), 
									  &(cmp -> nbO), &(act -> nbG), mort, k, i, 
									  nour_crea, nb_crea)))
					if (!mort)
						return 0;
				if ((!mort) && (act -> gar_p_tete) && (cmp -> gar_p_tete) &&
					(!fourmi_super_GG(&(act -> gar_p_tete), &(cmp -> gar_p_tete),
									  &(act -> nbG), &(cmp -> nbG), mort, i, k)))
						return 0;
			}
			k++;
			cmp = cmp -> precedent;
		}
		i++;
		k = 0;
		act = act -> precedent;
		cmp = dernier;
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée pour se placer au bout de la liste chainée
static void fourmiliere_fin (FOURMILIERE ** fml)
{
	if (* fml)
	{
		while ((* fml) -> suivant)
		{
			(* fml) = (* fml) -> suivant;
		}
	}
}

//*********************************************************************************//

//ajout d'un element FOURMILIERE dans la liste
static void fourmiliere_ajout (FOURMILIERE * nouv_fourmiliere)
{
	FOURMILIERE * nouv = NULL;
	
	if (!(nouv = (FOURMILIERE *) malloc(sizeof(FOURMILIERE))))
	{
		utilitaire_err_loc(__func__);
		exit (EXIT_FAILURE);
	}
	
	if ((nouv_fourmiliere -> nbO == 0) && (nouv_fourmiliere -> nbG == 0) 
		 && (nouv_fourmiliere -> total_food == 0))
	{
		nouv_fourmiliere -> vie = MORT;
	}
	else
		nouv_fourmiliere -> vie = VIVANT;
	
	//remplissage des données
	* nouv = * nouv_fourmiliere;
	
	//chainage de la structure
	nouv -> suivant = p_tete;
	nouv -> precedent = NULL;
	if (p_tete)
		nouv -> suivant -> precedent = nouv;
	p_tete = nouv;
	
	nouv -> ouv_p_tete = NULL;
	nouv -> gar_p_tete = NULL;
	nouv -> raid = 0;
}

//*********************************************************************************//

//suppression de la FOURMILIERE pointée par adr
void fourmiliere_supprime (FOURMILIERE * adr)
{
		//chainage
		FOURMILIERE * apres = adr -> suivant;
		FOURMILIERE * avant = adr -> precedent;
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
void fourmiliere_ecriture (FILE * sortie)
{
	FOURMILIERE * actuel = p_tete;
	int num = 0;
	
	while (actuel -> suivant)
	{
		actuel = actuel -> suivant;
	}
	
	while (actuel)
	{
		fprintf(sortie, "	#fourmiliere [%d]\n", num);
		fprintf(sortie, "	%lf %lf %d %d %lf %lf\n", actuel -> pos.x, 
													  actuel -> pos.y,
													  actuel -> nbO, 
													  actuel -> nbG,
													  actuel -> total_food, 
													  actuel -> rayon);
		if (actuel -> ouv_p_tete)
			fourmi_ouv_ecriture(sortie, actuel -> ouv_p_tete);
		if (actuel -> gar_p_tete)
			fourmi_gar_ecriture(sortie, actuel -> gar_p_tete);
		
		actuel = actuel -> precedent;
		num++;
	}
	if (p_tete)
		fprintf(sortie, "	FIN_LISTE\n");
	
}

//*********************************************************************************//

//suppression des données de la simulation actuelle
void fourmiliere_nettoyage (void)
{
	while (p_tete)
	{
		fourmi_nettoyage(p_tete -> ouv_p_tete, p_tete -> gar_p_tete);
		fourmiliere_supprime(p_tete);
	}
	nb_fourmiliere = NON_LU;
}

//*********************************************************************************//

//transfert des infos pour l'affichage GLUI
int fourmiliere_data_glui (int * tab, int * p_nbF, double * tab_rayon)
{
	FOURMILIERE * actuel = p_tete;
	int i = 0;
	
	if (p_tete == NULL)
		return 0;
		
	while (actuel -> suivant)
	{
		actuel = actuel -> suivant;
	}
	while (actuel)
	{
		if (i == MAX_FOURMILIERE * NB_INFOS)
			continue;
		tab_rayon[i / NB_INFOS] = actuel -> rayon;
		tab[i++] = (actuel -> nbO) + (actuel -> nbG);
		tab[i++] = actuel -> nbO;
		tab[i++] = actuel -> nbG;
		tab[i++] = actuel -> total_food;
		actuel = actuel -> precedent;
	}
	* p_nbF = nb_fourmiliere;
	
	return 1;
}

//*********************************************************************************//

//affichage des fourmilieres dans la fenetre GLUT
void fourmiliere_affichage (void)
{
	FOURMILIERE * actuel = p_tete;
	
	while (actuel)
	{
		if (actuel -> vie)
			graphic_dessin_cercle(actuel -> pos.x, actuel -> pos.y, actuel -> rayon, 
								  GRAPHIC_EMPTY, EPAIS, actuel -> num);
								  
		fourmi_affichage(actuel -> ouv_p_tete, actuel -> gar_p_tete, actuel -> num);
		actuel = actuel -> suivant;
	}
}

//*********************************************************************************//

//mise a jour des fourmilieres
void fourmiliere_update (int nb_nour, POINT * nour_pos, int * nour_cons, 
						 POINT ** nour_crea, int * nb_crea)
{
	FOURMILIERE * actuel = p_tete;
	POINT * tab_ouv_pos[MAX_FOURMILIERE], fml_pos[MAX_FOURMILIERE];
	int tab_nbO[MAX_FOURMILIERE], fml_vie[MAX_FOURMILIERE], i = 0;
	double fml_ray[MAX_FOURMILIERE], * fml_nour[MAX_FOURMILIERE];
	
	//recuperation d'infos sur les ouvrieres et les fourmilieres
	fourmiliere_info(tab_nbO, tab_ouv_pos, fml_ray, fml_pos, fml_vie, fml_nour);
	
	while (actuel)
	{
		if (actuel -> vie)
		{	
			//naissance de fourmi - consommation de nourriture - mise a jour du rayon
			fourmiliere_naissance(actuel);
			fourmiliere_nour_update(actuel);
			fourmiliere_rayon_update(actuel, fml_pos, fml_ray, fml_vie, i);
			
			//mise a jour des fourmis
			if (actuel -> ouv_p_tete)
				fourmi_ouv_update(&(actuel -> ouv_p_tete), actuel -> pos, 
								  actuel -> rayon, &(actuel -> nbO), 
								  &(actuel -> raid), nour_pos, nour_cons, nb_nour, 
								  &(actuel -> total_food), i, fml_vie, fml_ray, 
								  fml_pos, fml_nour);
			
			if (actuel -> gar_p_tete)
				fourmi_gar_update(&(actuel -> gar_p_tete), i, actuel -> pos, 
								  actuel -> rayon, tab_ouv_pos, tab_nbO,
								  &(actuel -> nbG));
								  
			//mort de la fourmiliere si necessaire
			if ((actuel -> total_food == 0) && (actuel -> gar_p_tete == NULL)
											&& (actuel -> ouv_p_tete == NULL))
			{
				actuel -> vie = MORT;
				actuel -> rayon = 0;
			}
		}
		actuel = actuel -> suivant;
		i++;
	}
	//morts prématurées des fourmis
	fourmiliere_superposition(OUI, nour_crea, nb_crea);
	
	for (i = 0 ; i < MAX_FOURMILIERE ; i++)			//liberation de la memoire
	{
		if (tab_ouv_pos[i])
			free(tab_ouv_pos[i]);
		tab_ouv_pos[i] = NULL;
	} 
}

//*********************************************************************************//

//recuperations d'information utiles a la maj de la simulation
static void fourmiliere_info (int * tab_ouv_nb, POINT ** tab_ouv_pos, 
							  double * fourmiliere_rayon, POINT * fourmiliere_pos,
							  int * fourmiliere_vie, double ** fourmiliere_nour)
{
	FOURMILIERE * actuel = p_tete;
	int i = 0;
	
	for (i = 0 ; i < MAX_FOURMILIERE ; i++)
	{
		tab_ouv_nb[i] = 0;
		tab_ouv_pos[i] = NULL;
		fourmiliere_rayon[i] = 0;
		fourmiliere_pos[i].x = 0;
		fourmiliere_pos[i].y = 0;
		fourmiliere_vie[i] = NON;
		fourmiliere_nour[i] = NULL;
		if (actuel)
		{
			fourmiliere_rayon[i] = actuel -> rayon;
			fourmiliere_pos[i] = actuel -> pos;
			fourmiliere_vie[i] = actuel -> vie;
			fourmiliere_nour[i] = &(actuel -> total_food);
			if (actuel -> nbO)
			{
				tab_ouv_nb[i] = actuel -> nbO;
				tab_ouv_pos[i] = calloc(actuel -> nbO, sizeof(POINT));
				fourmi_info_ouv(tab_ouv_pos[i], actuel -> ouv_p_tete);
			}
			actuel = actuel -> suivant;
		}
	}
}

//*********************************************************************************//

//consommation de nourriture
static void fourmiliere_nour_update (FOURMILIERE * actuel)
{
	double nbF = 0;
	
	nbF = (actuel -> nbO) + (actuel -> nbG);
	
	(actuel -> total_food) -= nbF * FEED_RATE;
	
	if (actuel -> total_food < VAL_FOOD)
		actuel -> total_food = 0;
		
}

//*********************************************************************************//

//mise a jour du rayon d'une fourmiliere
static void fourmiliere_rayon_update (FOURMILIERE * actuel, POINT * fourmiliere_pos,
									  double * fourmiliere_rayon, 
									  int * fourmiliere_vie, int num)
{
	double nouv_rayon = 0, dist = 0, dist_min = 0;
	int i = 0, nbF = 0, ind_min = UNDEF;
	
	nbF = (actuel -> nbO) + (actuel -> nbG);
	nouv_rayon = (1 + sqrt(nbF) + sqrt(actuel -> total_food)) * RAYON_FOURMI;
	
	if (nouv_rayon > (actuel -> rayon))
	{
		for (i = 0 ; i < MAX_FOURMILIERE ; i++)
		{
			dist = utilitaire_dist_2points(actuel -> pos, fourmiliere_pos[i]);
			if ((i != num) && (fourmiliere_vie[i]) &&
				(dist < fourmiliere_rayon[i] + nouv_rayon + EPSIL_ZERO))
			{
				if (ind_min == UNDEF)
				{
					dist_min = dist - fourmiliere_rayon[i] - nouv_rayon;
					ind_min = i;
				}
				else if (dist - fourmiliere_rayon[i] - nouv_rayon < dist_min)
				{
					dist_min = dist - fourmiliere_rayon[i] - nouv_rayon;
					ind_min = i;
				}
			}
		}
		if (ind_min == UNDEF)
			actuel -> rayon = nouv_rayon;
		else
			actuel -> rayon = 
				utilitaire_dist_2points(actuel -> pos, fourmiliere_pos[ind_min]) 
				- fourmiliere_rayon[ind_min] - EPSIL_ZERO;
	}
	else
		actuel -> rayon = nouv_rayon;
	
	fourmiliere_rayon[num] = actuel -> rayon;
}

//*********************************************************************************//

//gestion probabiliste des naissances de fourmis
static void fourmiliere_naissance (FOURMILIERE * actuel)
{
	int type = 0, nbF = 0;
	double ratio = 0;
	
	nbF = (actuel -> nbO) + (actuel -> nbG);
	
	if (utilitaire_prob() < (actuel -> total_food) * BIRTH_RATE)
	{
		if (nbF == 0)
		{
			type = OUV;
			(actuel -> nbO)++;
		}
		else
		{
			ratio = ((double) (actuel -> nbG)) / ((double) (nbF));
			if (ratio < RATIO_FOURMI)
			{
				type = GAR;
				(actuel -> nbG)++; 
			}
			else
			{
				type = OUV;
				(actuel -> nbO)++;
			}
		}
		fourmi_naissance (actuel -> pos, type, &(actuel -> gar_p_tete), 
						  &(actuel -> ouv_p_tete));
	}
}

//*********************************************************************************//

//test de superposition d'un element quelconque avec differents éléments
int fourmiliere_superposition_simple (POINT pos, double rayon, int option1, 
									  int option2, int option3)
{
	FOURMILIERE * actuel = p_tete;
	double dist = 0, delta = 0;
	
	while (actuel)
	{
		if ((option1) && (actuel -> vie))
		{
			dist = utilitaire_dist_2points(pos, actuel -> pos);
			delta = dist - rayon - (actuel -> rayon);
			
			if (delta < EPSIL_ZERO)
				return 0;
		}
		
		if ((option2) && (actuel -> vie) && 
			(fourmi_superposition_ouv_simple(pos, rayon, actuel -> ouv_p_tete) == 0))
			return 0;
		
		if ((option3) && (actuel -> vie) && 
			(fourmi_superposition_gar_simple(pos, rayon, actuel -> gar_p_tete) == 0))
			return 0;
		
		actuel = actuel -> suivant;
	}
	
	return 1;
}


