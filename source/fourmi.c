/*!
 \file fourmi.c
 \brief Module gerant les informations relatives aux fourmis.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "fourmi.h"
#include "error.h"
#include "constantes.h"
#include "utilitaire.h"
#include "graphic.h"

enum Symboles_fourmi {EPAIS = 2, NB_PARAM_GAR = 3, NB_PARAM_OUV = 6};
enum Type_fourmi {GAR, OUV};
enum Raid {MIN_DISPO = 7, MIN_NBO = 10, MIN_NOUR = 20};

typedef struct ouvriere OUVRIERE;
struct ouvriere
{
	long age;
	POINT pos;
	POINT but;
	int bool_nour;
	int raid;
	
	OUVRIERE * precedent;
	OUVRIERE * suivant;
};

typedef struct garde GARDE;
struct garde
{
	double age;
	POINT pos;
	POINT but;
	int no_but;
	
	GARDE * precedent;
	GARDE * suivant;
};

static int fourmi_gar_pos (int i, int j, POINT posF, POINT posC, double ray);
static int fourmi_fin_liste (FILE * fichier, int i, ERREUR_ORIG erreur);
static void fourmi_ouv_fin (OUVRIERE ** pp_ouv);
static void fourmi_gar_fin (GARDE ** pp_gar);
static void fourmi_choix_garde(GARDE * p_tete, POINT pos_ouv);

static void fourmi_raid (OUVRIERE ** adr_p_tete, int fml_nb, int * fml_vie, 
						 double * fml_ray, POINT * fml_pos, int nbO, int nb_nour,
						 int * raid);
static void fourmi_bon_choix (OUVRIERE * ouv, POINT * tab, int nb_nour, POINT centre,
							  int fml_num, int * fml_vie, double * fml_rayon,
							  POINT * fml_pos);
static void fourmi_nour_tombe (int * nb_crea, POINT ** nour_crea, POINT pos);

static int fourmi_trajectoire (POINT pos, POINT but, int fml_nb, POINT * fml_pos, 
							   int * fml_vie, double * fml_ray);
static void fourmi_super_OO_comp (int * i, int * mam, OUVRIERE ** act, 
								  OUVRIERE ** cmp, OUVRIERE * act_tete, 
								  OUVRIERE * cmp_tete, OUVRIERE ** dernier);
static void fourmi_super_OG_comp (int * i, int * mam, OUVRIERE ** act, GARDE ** cmp,
								  OUVRIERE * act_tete, GARDE * cmp_tete, 
							      GARDE ** dernier);
static void fourmi_super_GG_comp (int * i, int * mam, GARDE ** act, GARDE ** cmp,
								  GARDE * act_tete, GARDE * cmp_tete, 
								  GARDE ** dernier);
static void fourmi_ouv_contact (OUVRIERE ** adr_tete, POINT centre, POINT * nour_pos,
								int * nour_cons, int nb_nour_tot, double * nb_nour_f,
								int fml_nb, int * fml_vie, POINT * fml_pos, 
								double ** fml_nour);
static void fourmi_gar_depl (GARDE ** adr_tete, POINT pos, double rayon);

//*********************************************************************************//

//lecture des informations sur les ouvrières
int fourmi_lecture_ouv (FILE * fichier, int i, int nbO, OUVRIERE ** pp_tete)
{
	char mot[MAX_LINE], chaine[MAX_LINE];
	char * ligne = NULL;
	int j = 0, k = 0;
	OUVRIERE ouv_n;
	
	for (j = 0 ; j < nbO ; j++, k = 0)
	{	
		while ((!k) && (ligne = fgets(chaine, MAX_LINE, fichier)))
		{	
			if (utilitaire_debut_ligne(mot, chaine) == 0)
				continue;
			if (sscanf(chaine,"%ld %lf %lf %lf %lf %d", &(ouv_n.age),
													    &(ouv_n.pos.x),
													    &(ouv_n.pos.y),
													    &(ouv_n.but.x),
													    &(ouv_n.but.y),
													    &(ouv_n.bool_nour))
													    == NB_PARAM_OUV)
			{
				if (ouv_n.age >= BUG_LIFE)			//test de l'age des ouvrières
				{	
					error_age_fourmi(i, j, ouv_n.age);
					return 0;
				}
				fourmi_ouv_ajout(pp_tete, &ouv_n);
				k++;
			}
			else
			{
				error_lecture_elements_fourmiliere(i, ERR_OUVRIERE, ERR_PAS_ASSEZ);
				return 0;
			}
		}	
		if (utilitaire_EOF(ligne, __func__) == 0) return 0;
	}
	//verification de la presence de FIN_LISTE - elements en trop
	if ((nbO > 0) && (fourmi_fin_liste(fichier, i, ERR_OUVRIERE) == 0))
			return 0;
	
	return 1;
}

//*********************************************************************************//

//lecture des informations sur les gardes
int fourmi_lecture_gar (FILE * fichier, int i, int nbG, POINT posC, double ray, 
						GARDE ** pp_tete)
{
	char mot[MAX_LINE], chaine[MAX_LINE], fin_liste[] = {"FIN_LISTE"};
	char * debut = NULL, * fin = NULL, * ligne = NULL;
	int j = 0, k = 0, l = 0;
	double test_trop = 0;
	GARDE garde_n;
	
	while ((j < nbG) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0) 
			continue;
		if (strcmp(mot, fin_liste) == 0)
		{	
			error_lecture_elements_fourmiliere(i, ERR_GARDE, ERR_PAS_ASSEZ);
			return 0;
		}
		debut = chaine; 
		k = 0;
		while ((k == 0) && (sscanf(debut, "%lf %lf %lf", &(garde_n.age),
													     &(garde_n.pos.x),
													     &(garde_n.pos.y)) 
													     == NB_PARAM_GAR))
		{	//test de la presence des gardes au sein de leur fourmilière
			if (fourmi_gar_pos(i, j, garde_n.pos, posC, ray) == 0)
				return 0;
			//decalage du pointeur sur la ligne lue
			for (l = 0 ; l < NB_PARAM_GAR ; l++)
			{	
				strtod(debut, &fin);
				debut = fin;
			}
			fourmi_gar_ajout(pp_tete, &garde_n);
			j++;
			if (j > nbG-1) k++;
		}
	}
	if (nbG > 0)
	{
		if (utilitaire_EOF(ligne, __func__) == 0) 
			return 0;

		//verification elements en trop
		if (sscanf(debut, "%lf", &test_trop) == 1)
		{	
			error_lecture_elements_fourmiliere(i, ERR_GARDE, ERR_TROP);
			return 0;
		}
		//verification de la presence de FIN_LISTE - elements en trop
		if (fourmi_fin_liste(fichier, i, ERR_GARDE) == 0) 
			return 0;
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée testant la position d'un garde
static int fourmi_gar_pos (int i, int j, POINT posF, POINT posC, double ray)
{
	float dist = 0, delta = 0;
	
	dist = utilitaire_dist_2points(posF, posC);
	delta = ray - (dist + RAYON_FOURMI);
	
	if (delta < 0)
	{	
		error_pos_garde(i, j);
		return 0;
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée testant FIN_LISTE et les elements en trop
static int fourmi_fin_liste (FILE * fichier, int i, ERREUR_ORIG erreur)
{
	char mot[MAX_LINE], chaine[MAX_LINE], fin_liste[] = {"FIN_LISTE"};
	char * ligne = NULL;
	int k = 0;
	
	while ((!k) && (ligne = fgets(chaine, MAX_LINE, fichier)))
	{	
		if (utilitaire_debut_ligne(mot, chaine) == 0) 
			continue;
		if (strcmp(mot, fin_liste) != 0)
		{	
			error_lecture_elements_fourmiliere(i, erreur, ERR_TROP);
			return 0;
		}
		k++;
	}
	if (utilitaire_EOF(ligne, __func__) == 0) 
		return 0;
	return 1;
}

//*********************************************************************************//

//gestion des nourritures de fourmis tombées au combat
static void fourmi_nour_tombe (int * nb_crea, POINT ** nour_crea, POINT pos)
{	
	* nour_crea = (POINT *) realloc(* nour_crea, ++(* nb_crea) * sizeof(POINT));
	(* nour_crea)[* nb_crea - 1] = pos;
}

//*********************************************************************************//

//test des superpositions ouv / ouv
int fourmi_super_OO (OUVRIERE ** adr_actuel, OUVRIERE ** adr_compar,
					 int * nbO_actuel, int * nbO_compar, int mort, int num_fa, 
					 int num_fc, POINT ** nour_crea, int * nb_crea)
{
	OUVRIERE * ouv_actuel = * adr_actuel, * ouv_compar = * adr_compar;
	OUVRIERE * dernier = NULL;
	double dist = 0;
	int i = 0, k = 0, mam = NON;
	
	fourmi_ouv_fin(&(ouv_actuel));
	fourmi_ouv_fin(&(ouv_compar));
	dernier = ouv_compar;
	
	while (ouv_actuel)
	{
		mam = NON;
		k = 0;
		ouv_compar = dernier;
		while (ouv_compar)
		{
			dist = utilitaire_dist_2points(ouv_actuel -> pos, ouv_compar -> pos);
			if (dist < 2*RAYON_FOURMI + EPSIL_ZERO)
			{
				if (mort)
				{
					if (ouv_actuel -> bool_nour)
						fourmi_nour_tombe(nb_crea, nour_crea, ouv_actuel -> pos);
					if (ouv_compar -> bool_nour)
						fourmi_nour_tombe(nb_crea, nour_crea, ouv_compar -> pos);
					fourmi_ouv_supprime(adr_actuel, ouv_actuel);
					fourmi_ouv_supprime(adr_compar, ouv_compar);
					(*nbO_actuel)--;
					(*nbO_compar)--;
					ouv_actuel = NULL;
					ouv_compar = NULL;
					mam = OUI;
				}
				else
				{
					error_superposition_fourmi(ERR_OUVRIERE, num_fa, i,
											   ERR_OUVRIERE, num_fc, k);
					return 0;
				}
			}
			k++;
			if (!mam)
				ouv_compar = ouv_compar -> precedent;
		}
		fourmi_super_OO_comp(&i, &mam, &ouv_actuel, &ouv_compar, * adr_actuel,
							 * adr_compar, &dernier);
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée de complement au test de superposition ouv/ouv
static void fourmi_super_OO_comp (int * i, int * mam, OUVRIERE ** act, 
								  OUVRIERE ** cmp, OUVRIERE * act_tete, 
								  OUVRIERE * cmp_tete, OUVRIERE ** dernier)
{
	(* i)++;
	if (* mam)
	{
		* act = act_tete;
		* cmp = cmp_tete;
		fourmi_ouv_fin(act);
		fourmi_ouv_fin(cmp);
		* dernier = * cmp;
		i = 0;
	}
	else
		(* act) = (* act) -> precedent;
}

//*********************************************************************************//

//test des superpositions ouv / gar
int fourmi_super_OG (OUVRIERE ** adr_actuel, GARDE ** adr_compar, int * nbO_actuel, 
					  int * nbG_compar, int mort, int num_fa, int num_fc,
					  POINT ** nour_crea, int * nb_crea)
{
	OUVRIERE * ouv_actuel = * adr_actuel;
	GARDE * gar_compar = * adr_compar, * dernier = NULL;
	double dist = 0;
	int i = 0, k = 0, mam = NON;
	
	fourmi_ouv_fin(&(ouv_actuel));
	fourmi_gar_fin(&(gar_compar));
	dernier = gar_compar;
	
	while (ouv_actuel)
	{
		mam = NON;
		k = 0;
		gar_compar = dernier;
		while (gar_compar)
		{
			dist = utilitaire_dist_2points(ouv_actuel -> pos, gar_compar -> pos);
			if (dist < 2*RAYON_FOURMI + EPSIL_ZERO)
			{
				if (mort)
				{
					if (ouv_actuel -> bool_nour)
						fourmi_nour_tombe(nb_crea, nour_crea, ouv_actuel -> pos);
					fourmi_ouv_supprime(adr_actuel, ouv_actuel);
					fourmi_gar_supprime(adr_compar, gar_compar);
					(*nbO_actuel)--;
					(*nbG_compar)--;
					mam = OUI;
					ouv_actuel = NULL;
					gar_compar = NULL;
				}
				else
				{
					error_superposition_fourmi(ERR_OUVRIERE, num_fa, i,
											   ERR_GARDE, num_fc, k);
					return 0;
				}
			}
			k++;
			if (!mam)
				gar_compar = gar_compar -> precedent;
		}
		fourmi_super_OG_comp(&i, &mam, &ouv_actuel, &gar_compar, * adr_actuel,
							 * adr_compar, &dernier);
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée de complement au test de superposition ouv/gar
static void fourmi_super_OG_comp (int * i, int * mam, OUVRIERE ** act, GARDE ** cmp,
								  OUVRIERE * act_tete, GARDE * cmp_tete, 
							      GARDE ** dernier)
{
	(* i)++;
	if (* mam)
	{
		* act = act_tete;
		* cmp = cmp_tete;
		fourmi_ouv_fin(act);
		fourmi_gar_fin(cmp);
		* dernier = * cmp;
		i = 0;
	}
	else
		(* act) = (* act) -> precedent;
}

//*********************************************************************************//

//test des superpositions gar / gar
int fourmi_super_GG (GARDE ** adr_actuel, GARDE ** adr_compar, int * nbG_actuel, 
					 int * nbG_compar, int mort, int num_fa, int num_fc)
{
	GARDE * gar_actuel = * adr_actuel, * gar_compar = * adr_compar, * dernier = NULL;
	double dist = 0;
	int i = 0, k = 0, mam = NON;
	
	fourmi_gar_fin(&(gar_actuel));
	fourmi_gar_fin(&(gar_compar));
	dernier = gar_compar;
	
	while (gar_actuel)
	{
		mam = NON;
		k = 0;
		gar_compar = dernier;
		while (gar_compar)
		{
			dist = utilitaire_dist_2points(gar_actuel -> pos, gar_compar -> pos);
			if (dist < 2*RAYON_FOURMI + EPSIL_ZERO)
			{
				if (mort)
				{
					fourmi_gar_supprime(adr_actuel, gar_actuel);
					fourmi_gar_supprime(adr_compar, gar_compar);
					(*nbG_actuel)--;
					(*nbG_compar)--;
					mam = OUI;
					gar_actuel = NULL;
					gar_compar = NULL;
				}
				else
				{
					error_superposition_fourmi(ERR_GARDE, num_fa, i,
											   ERR_GARDE, num_fc, k);
					return 0;
				}
			}
			k++;
			if (!mam)
				gar_compar = gar_compar -> precedent;
		}
		fourmi_super_GG_comp(&i, &mam, &gar_actuel, &gar_actuel, * adr_actuel,
							 * adr_compar, &dernier);
	}
	return 1;
}

//*********************************************************************************//

//fonction non-exportée de complement au test de superposition ouv/gar    
static void fourmi_super_GG_comp (int * i, int * mam, GARDE ** act, GARDE ** cmp,
								  GARDE * act_tete, GARDE * cmp_tete, 
								  GARDE ** dernier)
{
	(* i)++;
	if (* mam)
	{
		* act = act_tete;
		* cmp = cmp_tete;
		fourmi_gar_fin(act);
		fourmi_gar_fin(cmp);
		* dernier = * cmp;
		i = 0;
	}
	else
		(* act) = (* act) -> precedent;
}

//*********************************************************************************//

//deplacement de pointeur en fin de liste chainée
static void fourmi_ouv_fin (OUVRIERE ** pp_ouv)
{
	if (* pp_ouv)
	{
		while ((* pp_ouv) -> suivant)
		{
			(* pp_ouv) = (* pp_ouv) -> suivant;
		}
	}
}

//*********************************************************************************//

//deplacement de pointeur en fin de liste chainée
static void fourmi_gar_fin (GARDE ** pp_gar)
{
	if (* pp_gar)
	{
		while ((* pp_gar) -> suivant)
		{
			(* pp_gar) = (* pp_gar) -> suivant;
		}
	}
}

//*********************************************************************************//

//ajout d'un element OUVRIERE dans la liste
void fourmi_ouv_ajout (OUVRIERE ** pp_tete, OUVRIERE * nouv_ouvriere)
{
	OUVRIERE * nouv = NULL;	
	
	if (!(nouv = (OUVRIERE *) malloc(sizeof(OUVRIERE))))
	{
		utilitaire_err_loc(__func__);
		exit (EXIT_FAILURE);
	}
	//remplissage des données
	* nouv = * nouv_ouvriere;
	nouv -> raid = NON;
	
	//chainage de la structure
	nouv -> suivant = * pp_tete;
	nouv -> precedent = NULL;
	if(* pp_tete)
		nouv -> suivant -> precedent = nouv;
	* pp_tete = nouv;

}

//*********************************************************************************//

//suppression de l'element OUVRIERE pointe par adr
void fourmi_ouv_supprime (OUVRIERE ** pp_tete, OUVRIERE * adr)
{
		//chainage
		OUVRIERE * apres = adr -> suivant;
		OUVRIERE * avant = adr -> precedent;
		if (avant == NULL)
			* pp_tete = adr -> suivant;
		else
			adr -> precedent -> suivant = apres;
		if (apres != NULL)
			adr -> suivant -> precedent = avant;
		
		//liberation de l'espace
		free(adr);
		adr = NULL;
}

//*********************************************************************************//

//ajout d'un element GARDE dans la liste
void fourmi_gar_ajout (GARDE ** pp_tete, GARDE * nouv_garde)
{
	GARDE * nouv = NULL;
	
	if (!(nouv = (GARDE *) malloc(sizeof(GARDE))))
	{
		utilitaire_err_loc(__func__);
		exit (EXIT_FAILURE);
	}
	//remplissage des données
	* nouv = * nouv_garde;
	
	//chainage de la structure
	nouv -> suivant = * pp_tete;
	nouv -> precedent = NULL;
	if(* pp_tete)
		nouv -> suivant -> precedent = nouv;
	* pp_tete = nouv;
}

//*********************************************************************************//

//suppression de l'element GARDE pointe par adr
void fourmi_gar_supprime (GARDE ** pp_tete, GARDE * adr)
{
		//chainage
		GARDE * apres = adr -> suivant;
		GARDE * avant = adr -> precedent;
		if (avant == NULL)
			* pp_tete = adr -> suivant;
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
void fourmi_ouv_ecriture (FILE * sortie, OUVRIERE * p_ouv_tete)
{
	OUVRIERE * actuel = p_ouv_tete;
	
	while (actuel -> suivant)
	{
		actuel = actuel -> suivant;
	}
	
	while (actuel)
	{
		fprintf(sortie, "		%ld %lf %lf %lf %lf %d\n", actuel -> age, 
														   actuel -> pos.x, 
														   actuel -> pos.y, 
														   actuel -> but.x,
														   actuel -> but.y, 
														   actuel -> bool_nour);
		
		actuel = actuel -> precedent;
	}
	
	if (p_ouv_tete)
		fprintf(sortie, "		FIN_LISTE\n");
}

//*********************************************************************************//

//ecriture des données de la simulation actuelle
void fourmi_gar_ecriture (FILE * sortie, GARDE * p_gar_tete)
{
	GARDE * actuel = p_gar_tete;
	
	while (actuel -> suivant)
	{
		actuel = actuel -> suivant;
	}
	
	while (actuel)
	{
		fprintf(sortie, "		%lf %lf %lf\n", actuel->age, 
												actuel->pos.x, 
												actuel->pos.y);
		
		actuel = actuel -> precedent;
	}
	
	if (p_gar_tete)
		fprintf(sortie, "		FIN_LISTE\n");
}

//*********************************************************************************//

//suppression des données de la simulation actuelle
void fourmi_nettoyage (OUVRIERE * p_ouv_tete, GARDE * p_gar_tete)
{
	while (p_ouv_tete)
	{
		fourmi_ouv_supprime(&p_ouv_tete, p_ouv_tete);
	}
	
	while (p_gar_tete)
	{
		fourmi_gar_supprime(&p_gar_tete, p_gar_tete);
	}
}

//*********************************************************************************//

//affichage des fourmis dans la fenetre GLUT
void fourmi_affichage (OUVRIERE * ouv_p_tete, GARDE * gar_p_tete, int num)
{
	OUVRIERE * ouv_actuel = ouv_p_tete;
	GARDE * gar_actuel = gar_p_tete;
	
	while (ouv_actuel)
	{
		graphic_dessin_cercle(ouv_actuel -> pos.x, ouv_actuel -> pos.y, RAYON_FOURMI,
							  GRAPHIC_EMPTY, EPAIS, num);
		if (ouv_actuel -> bool_nour == 1)
			graphic_dessin_cercle(ouv_actuel -> pos.x, ouv_actuel -> pos.y, 
								  RAYON_FOOD, GRAPHIC_EMPTY, EPAIS, NOIR);
		ouv_actuel = ouv_actuel -> suivant;
	}
	
	while (gar_actuel)
	{
		//remplissage
		graphic_dessin_cercle(gar_actuel -> pos.x, gar_actuel -> pos.y, RAYON_FOURMI,
							  GRAPHIC_FILLED, 0, num);
		//contour
		graphic_dessin_cercle(gar_actuel -> pos.x, gar_actuel -> pos.y, RAYON_FOURMI,
							  GRAPHIC_EMPTY, EPAIS, NOIR);
		gar_actuel = gar_actuel -> suivant;
	}
}

//*********************************************************************************//

//création de nouveau éléments OUVRIERE ou GARDE
void fourmi_naissance (POINT pos, int type, GARDE ** pp_gar_tete, 
					   OUVRIERE ** pp_ouv_tete)
{
	OUVRIERE ouv;
	GARDE gar;
	
	switch (type)
	{
		case GAR :
			gar.pos = pos;
			gar.age = 0;
			gar.suivant = NULL;
			gar.precedent = NULL;
			fourmi_gar_ajout(pp_gar_tete, &gar);
			break;
		case OUV :
			ouv.pos = pos;
			ouv.but = pos;
			ouv.age = 0;
			ouv.bool_nour = NON;
			ouv.raid = NON;
			ouv.suivant = NULL;
			ouv.precedent = NULL;
			fourmi_ouv_ajout(pp_ouv_tete, &ouv);
			break;
		default : 
			break;
	}
}

//*********************************************************************************//

//test de superposition d'un element quelconque avec les ouvrieres
int fourmi_superposition_ouv_simple (POINT pos, double rayon, OUVRIERE * p_tete)
{
	OUVRIERE * actuel = p_tete;
	double dist = 0, delta = 0;
	
	while (actuel)
	{
		dist = utilitaire_dist_2points(pos, actuel -> pos);
		delta = dist - rayon - RAYON_FOURMI;
		
		if (delta < EPSIL_ZERO)
			return 0;
		
		actuel = actuel -> suivant;
	}
	
	return 1;
}

//*********************************************************************************//

//test de superposition d'un element quelconque avec les gardes
int fourmi_superposition_gar_simple (POINT pos, double rayon, GARDE * p_tete)
{
	GARDE * actuel = p_tete;
	double dist = 0, delta = 0;
	
	while (actuel)
	{
		dist = utilitaire_dist_2points(pos, actuel -> pos);
		delta = dist - rayon - RAYON_FOURMI;
		
		if (delta < EPSIL_ZERO)
			return 0;
		
		actuel = actuel -> suivant;
	}
	
	return 1;
}

//*********************************************************************************//

//recuperation d'informations sur les ouvrieres
void fourmi_info_ouv (POINT * tab, OUVRIERE * p_tete)
{
	OUVRIERE * actuel = p_tete;
	int i = 0;
	
	while (actuel)
	{
		tab[i] = actuel -> pos;
		actuel = actuel -> suivant;
		i++;
	}
}

//*********************************************************************************//


//mise a jour des fourmis ouvrieres
void fourmi_ouv_update (OUVRIERE ** adr_p_tete, POINT centre, double rayon, 
						int * nbO, int * raid, POINT * nour_pos, int * nour_cons, 
						int nb_nour_tot, double * nb_nour_f, int fml_nb,
						int * fml_vie, double * fml_ray, POINT * fml_pos, 
						double ** fml_nour)
{
	OUVRIERE * ouv_actuel = * adr_p_tete;
	
	while (ouv_actuel)	//age des ouvrieres
	{
		ouv_actuel -> age ++;
		if (ouv_actuel -> age >= BUG_LIFE)
		{
			fourmi_ouv_supprime(adr_p_tete, ouv_actuel);
			(*nbO)--;
		}
		ouv_actuel = ouv_actuel -> suivant;
	}
	ouv_actuel = * adr_p_tete;
	
	//choix d'effectuer un raid
	if (++(*raid) >= MIN_RAID)
		fourmi_raid(adr_p_tete, fml_nb, fml_vie, fml_ray, fml_pos, *nbO, *nb_nour_f,
					raid);
	
	//choix du comportement + maj position
	while (ouv_actuel)
	{
		if ((ouv_actuel -> bool_nour) && (ouv_actuel -> raid))
		{
			ouv_actuel -> raid = NON;
			ouv_actuel -> but = centre;	
		}
		//possession nourriture -> retour centre fourmiliere dans tous les cas
		else if ((ouv_actuel -> bool_nour) || (!nb_nour_tot))
			ouv_actuel -> but = centre;
		else if (ouv_actuel -> raid);
		else 
			fourmi_bon_choix(ouv_actuel, nour_pos, nb_nour_tot, centre, fml_nb,
							 fml_vie, fml_ray, fml_pos);
		
		ouv_actuel -> pos = utilitaire_mouv(ouv_actuel -> pos, ouv_actuel -> but);
		ouv_actuel = ouv_actuel -> suivant;
	}
	
	fourmi_ouv_contact(adr_p_tete, centre, nour_pos, nour_cons, nb_nour_tot,
					   nb_nour_f, fml_nb, fml_vie, fml_pos, fml_nour);
	
}

//*********************************************************************************//

//test des contacts une fois le déplacement effectué
static void fourmi_ouv_contact (OUVRIERE ** adr_tete, POINT centre, POINT * nour_pos,
								int * nour_cons, int nb_nour_tot, double * nb_nour_f,
								int fml_nb, int * fml_vie, POINT * fml_pos, 
								double ** fml_nour)
{
	OUVRIERE * ouv_actuel = * adr_tete;
	double dist = 0;
	int i = 0;	
	
	while (ouv_actuel)
	{
		for (i = 0 ; i < nb_nour_tot ; i++) 	//contact avec nourriture
		{
			dist = utilitaire_dist_2points(ouv_actuel -> pos, nour_pos[i]);
			if ((dist < RAYON_FOOD + RAYON_FOURMI + EPSIL_ZERO)	
				&& (!ouv_actuel -> bool_nour) && (!nour_cons[i]))
			{
				nour_cons[i] = OUI;
				ouv_actuel -> bool_nour = OUI;
			}
		}
		if (ouv_actuel -> raid)					//vol de nourriture
		{
			for (i = 0 ; i < MAX_FOURMILIERE ; i++)
			{
				dist = utilitaire_dist_2points(ouv_actuel -> pos, fml_pos[i]);
				if ((dist < RAYON_FOURMI) && (i != fml_nb))
				{
					if (fml_vie[i])
					{
						ouv_actuel -> raid = NON;
						if (* (fml_nour[i]) >= VAL_FOOD)
						{
							* (fml_nour[i]) -= VAL_FOOD;
							ouv_actuel -> bool_nour = OUI;
						}
					}
					else
						ouv_actuel -> raid = NON;
				}
			}
		}
		if (ouv_actuel -> bool_nour) 			//contact centre de la fourmiliere
		{
			dist = utilitaire_dist_2points(ouv_actuel -> pos, centre);
			if (dist < RAYON_FOURMI)
			{
				ouv_actuel -> bool_nour = NON;
				(* nb_nour_f)++; 
			}	
		}
		ouv_actuel = ouv_actuel -> suivant;
	}
}

//*********************************************************************************//

//depart en croisade
static void fourmi_raid (OUVRIERE ** adr_p_tete, int fml_nb, int * fml_vie, 
						 double * fml_ray, POINT * fml_pos, int nbO, int nb_nour,
						 int * raid)
{
	OUVRIERE * actuel = * adr_p_tete;
	POINT cible_pos;
	int dispo = 0, cible = UNDEF, i = 0;
	double dist = 0, rayon_min = 0;
	
	while (actuel)								//ouvrieres disponibles pour le raid
	{
		dist = utilitaire_dist_2points(actuel -> pos, fml_pos[fml_nb]);
		if ((dist < fml_ray[fml_nb] - RAYON_FOURMI - EPSIL_ZERO)
			&& !(actuel -> bool_nour))
			dispo++;
		actuel = actuel -> suivant;
	}
	actuel = * adr_p_tete;
	
	for (i = 0 ; i < MAX_FOURMILIERE ; i++)		//contact avec d'autres fourmilieres
	{
		dist = utilitaire_dist_2points(fml_pos[fml_nb], fml_pos[i]);
		if ((i != fml_nb) && (fml_vie[i])
			&& (dist < fml_ray[fml_nb] + fml_ray[i] + EPSIL_ZERO + EPSIL_ZERO))
		{
			if (cible == UNDEF)
			{
				rayon_min = fml_ray[i];
				cible = i;
				cible_pos = fml_pos[i];
			}
			else if (fml_ray[i] < rayon_min)
			{
				rayon_min = fml_ray[i];
				cible = i;
				cible_pos = fml_pos[i];
			}
		}
	}
	//affectation de mission
	if ((dispo >= MIN_DISPO) && (nbO >= MIN_NBO) && (nb_nour >= MIN_NOUR) 
		&& (cible != UNDEF) && !(* raid = 0))
	{
		while (actuel)
		{
			dist = utilitaire_dist_2points(actuel -> pos, fml_pos[fml_nb]);
			if ((dist < fml_ray[fml_nb] - RAYON_FOURMI - EPSIL_ZERO)
				&& !(actuel -> bool_nour))
			{
				actuel -> but = cible_pos;
				actuel -> raid = OUI;
			}
			actuel = actuel -> suivant;
		} 
	}
}

//*********************************************************************************//

//choix de la nourriture la plus proche et accessible
static void fourmi_bon_choix (OUVRIERE * ouv, POINT * tab, int nb_nour, POINT centre,
							  int fml_nb, int * fml_vie, double * fml_ray,
							  POINT * fml_pos)
{
	int i = 0;
	double dist = 0, dist_min = UNDEF;
	
	for (i = 0 ; i < nb_nour ; i++)
	{
		dist = utilitaire_dist_2points(ouv -> pos, tab[i]);
		
		if (fourmi_trajectoire(ouv -> pos, tab[i], fml_nb, fml_pos, fml_vie, fml_ray)
			&& fourmi_trajectoire(fml_pos[fml_nb], tab[i], fml_nb, fml_pos, 
								  fml_vie, fml_ray))
		{
			if (dist_min == UNDEF) 
			{
				dist_min = dist;
				ouv -> but = tab[i];
			}
			else if (dist < dist_min)
			{
				dist_min = dist;
				ouv -> but = tab[i];
			}
		}
	}
	
	//pas de but accessible / pas de nourriture -> retour centre
	if (dist_min == UNDEF)
	{
		ouv -> but = centre;
	}
}				

//*********************************************************************************//

//mise a jour des fourmis gardes
void fourmi_gar_update (GARDE ** adr_p_tete, int num, POINT pos, double rayon, 
						POINT ** tab, int * nb, int * nbG)
{
	GARDE * gar_actuel = * adr_p_tete;
	int i = 0, k = 0;
	double dist = 0;
	
	while (gar_actuel)	//age des gardes
	{
		gar_actuel -> age ++;
		if (gar_actuel -> age >= BUG_LIFE)
		{
			fourmi_gar_supprime(adr_p_tete, gar_actuel);
			(*nbG)--;
		}
		gar_actuel = gar_actuel -> suivant;
	}
	gar_actuel = * adr_p_tete;
	
	//mettre a zero les directions
	while (gar_actuel)
	{
		gar_actuel -> no_but = OUI;
		gar_actuel = gar_actuel -> suivant;
	}
	
	//ouvriere etrangere dans la fourmiliere => garde le plus proche va vers elle
	for (i = 0 ; i < MAX_FOURMILIERE ; i++)
	{
		if ((i != num) && tab[i])
		{
			for (k = 0 ; k < nb[i] ; k++)
			{
				dist = utilitaire_dist_2points(pos, tab[i][k]);
				if (dist < rayon + RAYON_FOURMI - EPSIL_ZERO)
				{
					//choix du garde le plus proche et non-occupé
					fourmi_choix_garde(* adr_p_tete, tab[i][k]);
				}
			}
		}
	}
	
	fourmi_gar_depl(adr_p_tete, pos, rayon);
}

//*********************************************************************************//

//deplacement des gardes
static void fourmi_gar_depl (GARDE ** adr_tete, POINT pos, double rayon) 
{
	GARDE * gar_actuel = * adr_tete;
	POINT correction;
	double dist = 0, dist_corr = 0;
	
	while (gar_actuel)
	{
		//pas de cible => retour au centre de la fourmiliere
		if (gar_actuel -> no_but == OUI)
		{
			gar_actuel -> but = pos;
			gar_actuel -> no_but = NON;
		}
		//deplacement
		gar_actuel -> pos = utilitaire_mouv(gar_actuel -> pos, gar_actuel -> but);
		//correction
		dist = utilitaire_dist_2points(gar_actuel -> pos, pos);
		if (dist > rayon - RAYON_FOURMI - EPSIL_ZERO)
		{
			dist_corr = dist - rayon + RAYON_FOURMI + EPSIL_ZERO;
			if (dist_corr >= DELTA_T * BUG_SPEED)
				gar_actuel -> pos = utilitaire_mouv(gar_actuel -> pos, pos);
			else
			{
				correction.x = (dist_corr) * (pos.x - gar_actuel -> pos.x) / (dist);
				correction.y = (dist_corr) * (pos.y - gar_actuel -> pos.y) / (dist);
				gar_actuel -> pos = utilitaire_mouv(gar_actuel -> pos, correction);
			}
		}
		gar_actuel = gar_actuel -> suivant;
	}
}

//*********************************************************************************//

//choix du garde devant chasser l'intru
static void fourmi_choix_garde (GARDE * p_tete, POINT pos_ouv)
{
	GARDE * actuel = p_tete, * choisi = NULL;
	double dist_min = UNDEF, dist = 0;
	
	while (actuel)
	{
		dist = utilitaire_dist_2points(pos_ouv, actuel -> pos);
		
		if ((dist_min == UNDEF) && (actuel -> no_but == OUI))
		{
			actuel -> but = pos_ouv;
			actuel -> no_but = NON;
			choisi = actuel;
			dist_min = dist;
		}
		else if (actuel -> no_but == OUI)
		{
			if (dist < dist_min)
			{
				//définition d'une direction
				actuel -> but = pos_ouv;
				actuel -> no_but = NON;
				if (choisi)
					choisi -> no_but = OUI;
				choisi = actuel;
				dist_min = dist;
			}
		}
		actuel = actuel -> suivant;
	}
}

//*********************************************************************************//

//test de trajectoire (coupe une fourmiliere?)
static int fourmi_trajectoire (POINT pos, POINT but, int fml_num, POINT * fml_pos, 
							   int * fml_vie, double * fml_ray)
{
	double dist = 0, a = 0, b = 0, c = 0;
	int i = 0;
	
	a = utilitaire_dist_2points(pos, but);
	
	for (i = 0 ; i < MAX_FOURMILIERE ; i++)
	{
		b = utilitaire_dist_2points(pos, fml_pos[i]);
		c = utilitaire_dist_2points(but, fml_pos[i]);
		dist = sqrt(c*c - ((a*a - b*b + c*c)/(a+a)) * ((a*a - b*b + c*c)/(a+a)));
		
		if ((fml_vie[i]) && (i != fml_num) 
			&& (dist < fml_ray[i] + RAYON_FOURMI + EPSIL_ZERO)	  //"noeud papillon"
			&& !((c > b) && (a < c))							  //"bon coté"
			&& (a > b - fml_ray[i] - RAYON_FOURMI - EPSIL_ZERO))  //"au dela"
			
		{
			if ((a > b) || (c < fml_ray[i] + RAYON_FOURMI))		  //"dedans/dehors"
				return 0;
		}
	}
	
	return 1;
}


