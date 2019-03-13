/*!
 \file main.c
 \brief / Gère la programmation par evenement.
 \author 	Hartmann Florian
			Rimlinger Matthieu
 \version 3.0 (rendu final)
 \date 17 mai 2017
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>
#include <unistd.h>

//*********************************************************************************//

extern "C"
{
	#include "modele.h"
	#include "constantes.h"
}

//numeros des boutons
#define TEXTE1_ID 11
#define OPEN_BUTTON 12
#define TEXTE2_ID 13
#define SAVE_BUTTON 14
#define RADIO_FOOD_ID 21
#define START_BUTTON 31
#define STEP_BUTTON 32
#define CHECKBOX_RECORD_ID 33
#define EXIT_BUTTON 41
//autres
#define TAILLE_CHAINE 16

namespace 
{	
	//symboles
	enum Mode_lecture {ERROR, VERIF, GRAPHIC, FINAL, AUTRE};
	enum Arguments {NOM, MODE, FICHIER};
	enum Etat {OFF, ON};
	enum Options {NON, OUI};
	enum Chiffres {ZERO, UN, DEUX, TROIS};
	enum Statut {UNDEF = -1, MORT, VIVANT};
	enum Affichage {TOT_FOURMI, TOT_OUV, TOT_GAR, NB_FOOD, NB_INFOS};
	enum Symboles {TAILLE = 450, X_GLUT = 0, X_GLUI = 500, Y_GLUT = 0, Y_GLUI = 0};
	enum Couleurs {ROUGE, VERT, BLEU, ROSE, CYAN, JAUNE, BRUN, ORANGE, GRIS, VIOLET, 
				   NOIR, NB_COUL};
	
	static FILE * out = NULL, * food = NULL, * rayon = NULL;
	
	//variables boucle et d'etat
	int i = 0, j = 0, k = 0, cycle = 0;
	int mode = AUTRE, etat = OFF, arg = ON, nour = ON, record = OFF;
	
	//fenêtre de dessin, largeur et hauteur
	int fenetre_principale;
	int largeur,  hauteur;
	char chaine_1[TAILLE_CHAINE];
	char chaine_2[TAILLE_CHAINE];
	
	//informations pour l'affichage et le record
	int tab[MAX_FOURMILIERE][NB_INFOS];
	double tab_rayon[MAX_FOURMILIERE];
	int record_inf[MAX_FOURMILIERE];
	int nbF = 0, nbF_record = 0;
	
	//chaines de caracteres
	const char * couleur[NB_COUL] = {"Rouge", "Vert", "Bleu", "Rose", "Cyan", 
									 "Jaune", "Brun", "Orange", "Gris", "Violet"};
	const char * infos[NB_INFOS+1] = {"Couleur", "Fourmis total", "Ouvrieres",
									  "Gardes", "Nourriture"};
									  
	//variables pour Ortho
	GLfloat x_min, x_max, y_min, y_max, x_souris, y_souris;
	
	//interface GLUI
	GLUI * glui = NULL;
	GLUI_EditText * texte1 = NULL;
	GLUI_EditText * texte2 = NULL;
	GLUI_RadioGroup * radio_food = NULL;
	GLUI_Checkbox * checkbox_record = NULL;
	GLUI_Button * open_button = NULL;
	GLUI_Button * start_button = NULL;
	GLUI_Button * step_button = NULL;
	GLUI_StaticText * tab_glui[MAX_FOURMILIERE*NB_INFOS];
	GLUI_StaticText * tab_glui_total[NB_INFOS];
}

//fonctions
static void reshape_cb (int x, int y);
static void display_cb (void);
static void idle_cb (void);
static void maj_data_glui (void);
static void control_cb (int control);
static void fichier_cb (int control);
static void glui_creation (GLUI * glui);
static void glui_colonne1 (GLUI * glui);
static void glui_tableau (GLUI * glui, GLUI_Panel * principal_panel);
static void souris (int bouton, int etat, int x, int y);
static int id_mode (char const * mode);
static void record_fonc (void);
static void record_ecriture (void);

//*********************************************************************************//

//maintient des proportions dans la fenêtre GLUT
static void reshape_cb (int x, int y)
{
	glViewport(0, 0, x, y);
	
	largeur = x;
	hauteur = y;
	
	//Ajustement du domaine visualisé
	float aspect_ratio = (float) largeur/hauteur;
	
	if (aspect_ratio <= 1.)
	{
		x_min = - DMAX;
		x_max = DMAX;
		y_min = - DMAX / aspect_ratio;
		y_max = DMAX / aspect_ratio;
	}
	else
	{
		x_min = - DMAX * aspect_ratio;
		x_max = DMAX * aspect_ratio;
		y_min = - DMAX;
		y_max = DMAX;
	}

	glutPostRedisplay();
}

//*********************************************************************************//

//mise a jour de l'affichage GLUT avec OPEN_GL
static void display_cb (void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glLoadIdentity();
	glOrtho(x_min, x_max, y_min, y_max, -1.0, 1.0);
	
	//affichage de la simulation
	modele_affichage();
	
	glutSwapBuffers();
}

//*********************************************************************************//

//comportement du programme en l'absence d'action de l'utilisateur
static void idle_cb (void)
{	
	if (glutGetWindow() != fenetre_principale)
		glutSetWindow (fenetre_principale);
	
	if ((etat) && (mode == FINAL))
	{
		modele_update(nour);
		maj_data_glui();
		record_fonc();
	}
	else if ((etat) && (mode == GRAPHIC))
	{
		printf("modele_update\n");
	}
	
	glutPostRedisplay();
}

//*********************************************************************************//

//mise a jour des informations du tableau d'infos GLUI
static void maj_data_glui (void)
{
	char tab_char[TAILLE_CHAINE];
	int somme = 0;
	
	modele_data_glui(&tab[0][0], &nbF, tab_rayon);
	
	for (i = 0 ; i < nbF ; i++)
	{
		for (k = 0 ; k < NB_INFOS ; k++)
		{
			sprintf(tab_char, "%9d", tab[i][k]);
			tab_glui[i * NB_INFOS + k] -> set_text((char *) tab_char);
		}
	}
	for (i = 0 ; i < NB_INFOS ; i++)
	{
		somme = 0;
		for (k = 0 ; k < nbF ; k++)
		{
			somme += tab[k][i];
		}
		sprintf(tab_char, "%9d", somme);
		tab_glui_total[i] -> set_text((char *) tab_char);
	}
}

//*********************************************************************************//

//actions a effectuer selon l'action de l'utilisateur
static void control_cb (int control)
{
	switch (control)
	{
		case (RADIO_FOOD_ID):
			{
				if (radio_food -> get_int_val() == ON)
					nour = OFF;
				else 
					nour = ON;
			}
			break;
		case (START_BUTTON):
			if (etat)
			{
				start_button -> name = (char *) "Start !";
				etat = OFF;
				record = OFF;
				checkbox_record -> set_int_val(OFF);
				record_fonc();
			}
			else
			{
				start_button -> name = (char *) "Stop";
				etat = ON;
			}
			break;
		case (STEP_BUTTON):
			if (etat)
			{
				start_button -> name = (char *) "Start !";
				etat = OFF;
			}
			record = OFF;
			checkbox_record -> set_int_val(OFF);
			if (mode == GRAPHIC)
				printf("one step\nmodele_update\n");
			if (mode == FINAL)
			{
				modele_update(nour);
				maj_data_glui();
				record_fonc();
			}
			break;
		case (CHECKBOX_RECORD_ID):
			if (checkbox_record -> get_int_val())
				record = ON;
			else 
				record = OFF;
			break;
		case (EXIT_BUTTON):
			modele_nettoyage();
			break;
	}
}

//*********************************************************************************//

//callback ouverture et sauvegarde de fichier
static void fichier_cb (int control)
{
	switch (control)
	{
		case (OPEN_BUTTON):
			{
				if (etat == OFF)
				{
					modele_nettoyage();
					if (modele_lecture(GRAPHIC, chaine_1) == 0)
						modele_nettoyage();
					GLUI_Master.close_all();
					etat = OFF;
					record = OFF;
					nour = ON;
					glui = GLUI_Master.create_glui((char *) "Bug's Life", 0, X_GLUI, 
																			 Y_GLUI);
					glui_creation(glui);
				}
			}
			break;
		case (SAVE_BUTTON):
			{
				char vide[] = {""};
				if (strcmp(vide, chaine_2) == 0)
				{
					printf("erreur : impossible de créer le fichier de "
						   "sauvegarde\n");
					GLUI_Master.close_all();
					glutDestroyWindow(fenetre_principale);
				}
				else
					modele_ecriture(chaine_2);
			}
			break;
	}
}

//*********************************************************************************//

//creation de l'interface GLUI
static void glui_creation (GLUI * glui)
{	
	char tab_char[TAILLE_CHAINE];
	
	//premiere colonne de la fenetre	
	glui_colonne1(glui);
	
	//deuxieme colonne
	GLUI_Panel * principal_panel = glui -> add_rollout((char *) "Information", true);
	GLUI_Panel * en_tete_panel = glui -> add_panel_to_panel(principal_panel,"", 
														    GLUI_PANEL_NONE);
	
	for (i = 0 ; i <= NB_INFOS ; i++)
	{
		glui -> add_statictext_to_panel(en_tete_panel, (char *) infos[i]);
		if (i < NB_INFOS)
			glui -> add_column_to_panel(en_tete_panel, false);
	}
	
	//mise a jour des informations et remplissage du tableau
	glui_tableau(glui, principal_panel);
	
	//panel TOTAL
	GLUI_Panel * total_panel = glui -> add_panel_to_panel(principal_panel, "", 
														  GLUI_PANEL_NONE);
	glui -> add_statictext_to_panel(total_panel, (char *) "Total");
	glui -> add_column_to_panel(total_panel, false);
	for (i = 0 ; i < NB_INFOS ; i++)
	{
		int somme = 0;
		for (k = 0 ; k < MAX_FOURMILIERE ; k++)
		{
			somme += tab[k][i];
		}
		sprintf(tab_char, "%9d", somme);
		tab_glui_total[i] = 
			glui -> add_statictext_to_panel(total_panel, (char *) tab_char);
		glui -> add_column_to_panel(total_panel, false);
	}
}

//*********************************************************************************//

//creation de l'interface GLUI - premiere colonne
static void glui_colonne1 (GLUI * glui)
{
	//ouverture et sauvegarde de fichier
	GLUI_Panel * file_panel = glui -> add_panel((char *) "Object Type", 
											    GLUI_PANEL_EMBOSSED);
											   
	texte1 = glui -> add_edittext_to_panel(file_panel, (char *) "FileName:", 
										   GLUI_EDITTEXT_TEXT, chaine_1, TEXTE1_ID);
										   
	open_button = glui -> add_button_to_panel(file_panel, (char *) "OPEN", 
											  OPEN_BUTTON, fichier_cb);
											  
	texte2 = glui -> add_edittext_to_panel(file_panel, (char *) "FileName:", 
										   GLUI_EDITTEXT_TEXT, chaine_2, TEXTE2_ID);
										   
	glui -> add_button_to_panel(file_panel, (char *) "SAVE", 
								SAVE_BUTTON, fichier_cb);
								
	if (arg == OFF)
	{
		arg = ON;
		texte1 -> set_text("test.txt");
	}
	texte2 -> set_text("save.txt");
					
	//creation de nourriture
	GLUI_Panel * food_creation_panel = glui -> add_panel((char *) "FoodCreation", 
														 GLUI_PANEL_EMBOSSED);
	radio_food = glui -> add_radiogroup_to_panel(food_creation_panel, NULL, 
											     RADIO_FOOD_ID, control_cb);

	glui -> add_radiobutton_to_group(radio_food,(char *) "Automatic");
	glui -> add_radiobutton_to_group(radio_food,(char *) "Manual");
	
	//gestion de la simulation
	GLUI_Panel * simulation_panel = glui -> add_panel((char *) "Simulation", 
													  GLUI_PANEL_EMBOSSED);
													 
	start_button = glui -> add_button_to_panel(simulation_panel, (char *) "Start !", 
											   START_BUTTON, control_cb);
											   
	step_button = glui -> add_button_to_panel(simulation_panel, (char *) "Step", 
											  STEP_BUTTON, control_cb);
											  
	checkbox_record = glui -> add_checkbox_to_panel(simulation_panel, 
													(char *) "Record", NULL, 
													CHECKBOX_RECORD_ID, control_cb);
	//bouton EXIT
	glui -> add_button((char *) "Exit", EXIT_BUTTON, (GLUI_Update_CB)exit);
	
	glui -> add_column(false);
}

//*********************************************************************************//

//creation de l'interface GLUI - remplissage du tableau
static void glui_tableau (GLUI * glui, GLUI_Panel * principal_panel)
{
	GLUI_Panel * info_panel = glui -> add_panel_to_panel(principal_panel, "", 
														 GLUI_PANEL_NONE);
														
	modele_data_glui(&tab[0][0], &nbF, tab_rayon);
	
	char tab_char[TAILLE_CHAINE];
	
	if (nbF <= 0)
		glui -> add_statictext_to_panel(info_panel, (char*) "");
	
	for (i = 0 ; i <= NB_INFOS ; i++)					//colonnes
	{
		k = 0;
		while (k < nbF)									//lignes
		{
			if (i == 0)
			{
				glui -> add_statictext_to_panel(info_panel, (char *) "");
				glui -> add_statictext_to_panel(info_panel, (char *) couleur[k]);
			}
			else
			{
				sprintf(tab_char, "%9d", tab[k][i-1]);
				glui -> add_statictext_to_panel(info_panel, (char *) "");
				tab_glui[(i-1) + NB_INFOS * k] = 
					glui -> add_statictext_to_panel(info_panel, (char *) tab_char);
			}
			k++;
		}
		glui -> add_column_to_panel(info_panel, false);
	}
	glui -> add_separator_to_panel(principal_panel);
	nbF = 0;
}

//*********************************************************************************//

//determine le mode de test
static int id_mode (char const * mode)
{
	char const * mode_n[AUTRE] = {"Error", "Verification", "Graphic", "Final"};
	
	if (strcmp(mode, mode_n[ERROR]) == 0)
		return ERROR;
	else if (strcmp(mode, mode_n[VERIF]) == 0)
		return VERIF;
	else if (strcmp(mode, mode_n[GRAPHIC]) == 0)
		return GRAPHIC;
	else if (strcmp(mode, mode_n[FINAL]) == 0)
		return FINAL;
	else
		return AUTRE;
}

//*********************************************************************************//

//gestion des évenements de la souris
static void souris (int bouton, int etat, int x, int y)
{
	x_souris = (((double) (x))/largeur) * (x_max - x_min) + x_min;
	y_souris = y_max - (y_max - y_min) * (((double) (y))/hauteur);
	
	if ((radio_food -> get_int_val() == ON) && (bouton == GLUT_RIGHT_BUTTON))
		modele_nour_gen(OUI, x_souris, y_souris);
	
}

//*********************************************************************************//

//enregistrement de l'évolution pour visualisation
static void record_fonc (void)
{	
	if ((record) && (out != NULL))							//record en cours
	{
		if (nbF_record)
			record_ecriture();
		cycle++;
	}
	else if ((record) && (out == NULL))						//debut de record
	{
		for (i = 0 ; i < MAX_FOURMILIERE ; i++)
		{
			if (tab[i][TOT_FOURMI] || tab[i][NB_FOOD])
			{
				record_inf[i] = VIVANT;
				nbF_record++;
			}
			else
				record_inf[i] = MORT;
		}
		
		out = fopen("out.dat", "w");
		food = fopen("food.dat", "w");
		rayon = fopen("rayon.dat", "w");
		if (nbF_record)
			record_ecriture();
		cycle++;
	}
	else if ((!record) && (out))							//fin de record
	{
		fclose(out);
		fclose(food);
		fclose(rayon);
		out = NULL;
		food = NULL;
		rayon = NULL;
		cycle = 0;
		printf("out.dat : written for %d fourmiliere\n", nbF_record);
		printf("food.dat : written for %d fourmiliere\n", nbF_record);
		printf("rayon.dat : written for %d fourmiliere\n", nbF_record);
		nbF_record = 0;
	}
}

//*********************************************************************************//

//enregistrement de l'évolution pour visualisation
static void record_ecriture (void)
{
	fprintf(out, "%d ", cycle);
	fprintf(food, "%d ", cycle);
	fprintf(rayon, "%d ", cycle);
	
	for (i = 0 ; i < nbF ; i++)
	{
		if (record_inf[i])
		{
			fprintf(out, "%d ", tab[i][TOT_FOURMI]);
			fprintf(food, "%d ", tab[i][NB_FOOD]);
			fprintf(rayon, "%.5lf ", tab_rayon[i]);
		}
	}
	
	fprintf(out, "\n");
	fprintf(food, "\n");
	fprintf(rayon, "\n");
}

//*********************************************************************************//

//MAIN
int main (int argc, char * argv[])
{	
	if (argc == UN)
	{
		arg = OFF;
		mode = FINAL;
	}
	else if (argc == TROIS)
	{
		sprintf(chaine_1, "%s", argv[DEUX]);
		mode = id_mode(argv[MODE]);
		if (mode == AUTRE)
		{
			printf("erreur : mode de test inutilisé\n");
			return EXIT_FAILURE;
		}
		if (modele_lecture(mode, argv[FICHIER]) == 0)
		{
			modele_nettoyage();
			if (mode != GRAPHIC)
				return EXIT_FAILURE;
		}
		if ((mode == ERROR) || (mode == VERIF))
			return EXIT_SUCCESS;
	}
	else
	{
		printf("erreur : usage\n");
		return EXIT_FAILURE;
	}
	
	//initialisation de la fenêtre GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(X_GLUT,Y_GLUT);
	glutInitWindowSize(TAILLE, TAILLE);
	fenetre_principale = glutCreateWindow("Bug's Life");
	glutDisplayFunc(display_cb);
	glutReshapeFunc(reshape_cb);
	
	//initialisation souris
	GLUI_Master.set_glutMouseFunc(souris);
	
	//initialisation et creation de l'interface graphique utilisateur GLUI
	glui = GLUI_Master.create_glui((char *) "Bug's Life", NON, X_GLUI, Y_GLUI);
	glui_creation(glui);
	
	glui -> set_main_gfx_window( fenetre_principale );
	GLUI_Master.set_glutReshapeFunc( reshape_cb );
	GLUI_Master.set_glutIdleFunc( idle_cb );
	glutMainLoop();
		
	return EXIT_SUCCESS;
}

