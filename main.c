#include "shell.h"

int main()
{
    char ***env = initEnv(MAX_CUSTOM_ENV); //Allocation dynamique dans la mémoire d'une matrice de chaîne de caractères pour conserver les variables d'environnement de l'utilisateur
    char *str; //Déclaration d'un tampon en tant que chaîne de caractères
    char cwd[1024]; //Déclaration d'une chaîne de caractères pour stocker le répertoire actuel
    int term_status = 0; //Variable entraînant la fin du programme si elle est égale à -1
    str = (char *)calloc(ARG_MAX, sizeof(char)); //Allocation dynamique dans la mémoire d'un tampon pour stocker l'entrée de l'utilisateur
    History *head = NULL; // Début de l'historique
    History *tail = NULL;

    while (term_status != -1) //Tant que le statut du terminal n'est pas égal à -1
    {
        if (getcwd(cwd, sizeof(cwd)) != NULL) { //Le programme essaye de récupérer le répertoire actuel
            printf("\033[36m%s ", cwd); //S'il réussit, il l'affiche
        }

        printf("\033[0m> ");                            // Afficher le prompt
        lireCommande(str);                       // Lire la commande utilisateur
        add_history(&head, &tail, str); //Ajout de la commande à l'historique
        PipelineOrDirect(str, env, head, &term_status); //Envoi de la commande de l'utilisateur à la procédure principale
    } // Si le statut du terminal est égal à -1 alors
    free_history(&head); // Libération dans la mémoire de l'historique
    freeEnv(env, MAX_CUSTOM_ENV); // Libération dans la mémoire des variables d'environnement
    free(str); //Libération dans la mémoire du tampon
    return 0; //Sortie du programme
}