#include <stdio.h> // Nécessaire pour gérer les IOs
#include <stdlib.h>
#include <dirent.h>    // Manipulation des répertoires
#include <unistd.h>    // Pour getcwd()
#include <errno.h>     // Pour gérer les erreurs
#include <string.h>    // Pour strcpy
#include <stdbool.h>   // Pour les boolean
#include <sys/types.h> // Pour la gestion des PID
#include <sys/wait.h>  // Inclus pour la fonction wait()
#include <sys/stat.h>
//
// Created by daf on 15/10/24.
//t

#define ARG_MAX 2097152
#define MAX_CUSTOM_ENV 250
#define ENV_MAX 1024
#define PARSE_ARG_MAX 1024

#ifndef PROJETC3A_SHELL_H
#define PROJETC3A_SHELL_H

// Structure pour stocker une ligne de commande
typedef struct History
{
    int index;
    char *line;
    struct History *back;
    struct History *next;
} History;


// Déclaration des pointeurs externes pour accéder à l'historique
extern History *current;

void lireCommande(char *buffer);
void parserCommande(char *buffer, char **arguments);

void gererCommande(char **arguments, char ***env, History **head, int* term_status);
int executerCommandeInterne(char **arguments);
int gestionEnvironnement(char **arguments, char ***env);

void add_history(History **head, History **tail, const char *command);
void print_history(History *head);
void free_history(History **head);
char ***initEnv(const int max);
bool updateEnv(char *name, char *value, char ***env);
void addEnv(char *name, char *value, char ***env);
void printEnv(char ***env);
void freeEnv(char ***env, int max);


void parserCommande2(char* line, char** args);
//void gererPipeline(char* commande, char ***env, History *head, History *tail);
void gererPipeline(char** args);
void PipelineOrDirect(char* commande, char ***env, History *head, int* term_status);
char* remove_first_and_last(char** arguments, int i);
#endif // PROJETC3A_SHELL_H
