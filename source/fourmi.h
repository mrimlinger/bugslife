/*!
 \file fourmi.h
 \brief Module gerant les informations relatives aux fourmis.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#ifndef FOURMI_H
#define FOURMI_H

#include "utilitaire.h"

typedef struct ouvriere OUVRIERE;
typedef struct garde GARDE;

//lecture des informations sur les ouvrières
int fourmi_lecture_ouv (FILE * fichier, int i, int nbO, OUVRIERE ** pp_tete);

//lecture des informations sur les gardes
int fourmi_lecture_gar (FILE * fichier, int i, int nbG, POINT posC, double ray, 
						GARDE ** pp_tete);
					    
//fonctions d'ajout et de suppression de fourmis
void fourmi_ouv_ajout (OUVRIERE ** pp_tete, OUVRIERE * nouv_ouvriere);
void fourmi_ouv_supprime (OUVRIERE ** pp_tete, OUVRIERE * adr);

void fourmi_gar_ajout (GARDE ** pp_tete, GARDE * nouv_garde);
void fourmi_gar_supprime (GARDE ** pp_tete, GARDE * adr);

//verifications des superpositions interdites entre fourmis				    
int fourmi_super_OO (OUVRIERE ** adr_actuel, OUVRIERE ** adr_compar,
					 int * nbO_actuel, int * nbO_compar, int mort, int num_fa, 
					 int num_fc, POINT ** nour_crea, int * nb_crea);
int fourmi_super_OG (OUVRIERE ** adr_actuel, GARDE ** adr_compar, int * nbO_actuel, 
					  int * nbG_compar, int mort, int num_fa, int num_fc,
					  POINT ** nour_crea, int * nb_crea);
int fourmi_super_GG (GARDE ** adr_actuel, GARDE ** adr_compar, int * nbG_actuel, 
					 int * nbG_compar, int mort, int num_fa, int num_fc);
							  
//ecriture des données de la simulation actuelle
void fourmi_ouv_ecriture (FILE * sortie, OUVRIERE * p_ouv_tete);
void fourmi_gar_ecriture (FILE * sortie, GARDE * p_gar_tete);

//suppression des données de la simulation actuelle
void fourmi_nettoyage (OUVRIERE * p_ouv_tete, GARDE * p_gar_tete);

//affichage des fourmis dans la fenetre GLUT
void fourmi_affichage (OUVRIERE * ouv_p_tete, GARDE * gar_p_tete, int num);

//création de nouveau éléments OUVRIERE ou GARDE
void fourmi_naissance (POINT pos, int type, GARDE ** pp_gar_tete, 
					   OUVRIERE ** pp_ouv_tete);

//test de superposition d'un element quelconque avec les ouvrieres
int fourmi_superposition_ouv_simple (POINT pos, double rayon, OUVRIERE * p_tete);

//test de superposition d'un element quelconque avec les gardes
int fourmi_superposition_gar_simple (POINT pos, double rayon, GARDE * p_tete);

//recuperation d'informations sur les ouvrieres
void fourmi_info_ouv (POINT * tab, OUVRIERE * p_tete);

//mise a jour des fourmis ouvrieres
void fourmi_ouv_update (OUVRIERE ** adr_p_tete, POINT centre, double rayon, 
						int * nbO, int * raid, POINT * nour_pos, int * nour_cons, 
						int nb_nour_tot, double * nb_nour_f, int fml_nb,
						int * fml_vie, double * fml_ray, POINT * fml_pos,
						double ** fml_nour);

//mise a jour des fourmis gardes
void fourmi_gar_update (GARDE ** adr_p_tete, int num, POINT pos, double rayon, 
						POINT ** tab, int * nb, int * nbG);

#endif

