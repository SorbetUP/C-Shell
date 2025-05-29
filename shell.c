#include "shell.h"

void lireCommande(char *buffer)
{
    while (fgets(buffer, ARG_MAX, stdin) != NULL)
    { // On lit stdin (entrée standard) et on en affecte les ARG_MAX-1 caractères à buffer
        if (buffer[strlen(buffer) - 1] == '\n')
        // Étant donné que la touche Entrée retourne généralement un caractère de nouvelle ligne (\n).
        // strlen(buffer)-1 nous positionne au dernier caractère de buffer.
        {
            buffer[strlen(buffer) - 1] = '\0';
            // On le remplace par un caractère de fin de string (\0)
            break;
            // On quitte la boucle
        }
    }
}


void parserCommande(char* line, char** args) {
    int i = 0, line_idx = 0, col_idx = 0;
    int in_quotes = 0;
    int in_parenthese = 0;

    while (line[i] != '\0') {
        // Fait toute les conditions de parsing
        // Si on rentre entre 2 parenthese ou 2 guillemet alors les espaces,
        //  | , < et > ne font plus saute de ligne
        if (line[i] == '"' && !in_quotes && !in_parenthese) {
            in_quotes = 1;
            args[line_idx][col_idx] = '\"';
            col_idx++;

        } else if (line[i] == '"' && in_quotes && !in_parenthese) {
            in_quotes = 0;
            args[line_idx][col_idx] = '\"';
            col_idx++;

        } else if (line[i] == '(' && !in_quotes && !in_parenthese) {
            in_parenthese = 1;
            args[line_idx][col_idx] = '(';
            col_idx++;

        } else if (line[i] == ')' && !in_quotes && in_parenthese) {
            in_parenthese = 0;
            args[line_idx][col_idx] = ')';
            col_idx++;

        } else if (line[i] == ' ' && !in_quotes  && !in_parenthese) {
            if (col_idx > 0) { // fin d'un argument
                args[line_idx][col_idx] = '\0';
                line_idx++;
                col_idx = 0;
            }
        } else if (line[i] == '|' && !in_quotes  && !in_parenthese) {
            if (col_idx > 0) {
                args[line_idx][col_idx] = '\0';
                line_idx++;
            }
            args[line_idx][0] = '|';
            args[line_idx][1] = '\0';
            line_idx++;
            col_idx = 0;
        } else if (line[i] == '>' && !in_quotes  && !in_parenthese) {
            if (col_idx > 0) {
                args[line_idx][col_idx] = '\0';
                line_idx++;
            }
            args[line_idx][0] = '>';
            args[line_idx][1] = '\0';
            line_idx++;
            col_idx = 0;
        } else {
            args[line_idx][col_idx++] = line[i];
        }
        i++;
    }
    
    if (col_idx > 0) { // Dernier argument
        args[line_idx][col_idx] = '\0';
        line_idx++;
    }
    args[line_idx] = NULL; // On termine la liste d'argument par NULL pour s'inifier la fin
}

void executerCommande(char **arguments)
{
    if (arguments == NULL || arguments[0] == NULL)
    {
        fprintf(stderr, "Erreur aucune commande spécifiée \n");
        // Vérification si c'est vide. Si oui alors, on sort de la fonction
        return;
    }

    int background = 0; // Variable pour déterminer si la commande est en arrière-plan
    // Vérifier si le dernier argument est "&" pour le mode arrière-plan
    int i = 0;
    while (arguments[i] != NULL)
    {
        i++; // Compte du nombre d'éléments présents dans les arguments
    }

    if (i > 0 && strcmp(arguments[i - 1], "&") == 0) // il y a plus d'un argument et que l'argument & se trouve a la fin
    {
        background = 1;          // La commande doit être exécutée en arrière-plan
        arguments[i - 1] = NULL; // Retirer "&" des arguments pour execvp
    }

    // Création d'un processus enfant
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Erreur lors du fork()");
        // Vérifier que le fork s'est bien initialisé et retourner l'erreur si ce n'est pas le cas
        return;
    }

    if (pid == 0)
    {
        // Processus enfant : exécuter la commande
        if (execvp(arguments[0], arguments) == -1) // si pas de -1 alors la commande s'est bien executée
        {
            perror("Erreur lors de l'exécution de la commande");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (background)
        {
            printf("Commande en arrière-plan lancée avec comme PID : %d\n", pid); // affiche l'identifiant du processus enfant
        }
        else
        {
            // Processus en premier plan : attendre la fin de l'enfant
            int status;
            if (waitpid(pid, &status, 0) == -1) // attend la fin des processus enfant
            {
                perror("Erreur lors de waitpid()");
                return;
            }

            // Analyse du statut du processus enfant sans utiliser WIFEXITED ou WIFSIGNALED
            if (status == 0) // bonne execution
            {
            }
            else // erreur
            {
                if (status > 0 && status <= 255) // entre 1 et 255, soit il s'agit d'une erreur normale, soit cela vient d'un processus exterieur
                {
                    printf("Le processus enfant s'est terminé avec un code d'erreur : %d.\n", status);
                }
                else
                {
                    printf("Le processus enfant a été interrompu par un événement externe ou un signal.\n");
                }
            }
        }
    }
}
// Execute les commandes internes
int executerCommandeInterne(char **arguments)
{
    if (arguments == NULL || arguments[0] == NULL)
    {
        fprintf(stderr, "Erreur : aucune commande spécifiée.\n");
        // S'il n'y a aucune commande donnée en arguments, rien ne peut être exécutée
        return 0;
    }

    // Commande : cd
    if (strcmp(arguments[0], "cd") == 0) // Vérifier si la commande est cd
    {
        if (arguments[1] == NULL) // Si la commande n'est suivi d'aucun argument
        {
            // Récupérer dans les variables d'environnement le dosser home de l'utilisateur, soit /home/<username>
            const char *home = getenv("HOME");
            if (home != NULL) {
                if (chdir(home) != 0) // Accéder à son dossier home
                {
                    perror("Erreur lors du changement de répertoire");
                }
            } else {
                fprintf(stderr, "Erreur : la variable d'environnement HOME n'est pas définie.\n");
                // Renvoyer une erreur si la variable d'environnement HOME n'est pas définie, bien qu'elle l'est par défaut sur Linux
            }
            //fprintf(stderr, "Erreur : chemin non spécifié pour cd.\n");
        }
        else if (chdir(arguments[1]) != 0) // S'il y a un argument donné avec cd, alors essai de déplacement dans ce répertoire. Exemple cd DossierDuChat
        {
            perror("Erreur lors du changement de répertoire");
        }
        return 1;
    }

    // Commande : exit
    if (strcmp(arguments[0], "exit") == 0)
    {
        return(-1); // Code de sortie si l'utilisateur entre exit
    }

    // Commande : pwd
    if (strcmp(arguments[0], "pwd") == 0)
    {
        char cwd[1024]; // Tableau de 1024 char
        if (getcwd(cwd, sizeof(cwd)) != NULL) // Récupérer dans cwd le chemin pwd
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("Erreur lors de la récupération du répertoire courant");
        }
        return 1;
    }

    // Commande : echo
    if (strcmp(arguments[0], "echo") == 0)
    {
        for (int i = 1; arguments[i] != NULL; i++)
        {
            char* str = remove_first_and_last(arguments, i);
            // Retirer les guillemets ("") s'il y en a. Exemple echo "chat rouge" n'affichera que chat rouge
            printf("%s ", arguments[i]);
            free(str); // Libérer dans la mémoire str
        }
        printf("\n");
        return 1;
    }


    // Commande : mkdir
    if (strcmp(arguments[0], "mkdir") == 0)
    {
        char* str = remove_first_and_last(arguments, 1);
        // Pareil que pour echo, retire les guillemets sinon ne fait rien
        if (str == NULL || *str == '\0')
        {
            fprintf(stderr, "Erreur : chemin non spécifié pour mkdir.\n");
        }
        else if (mkdir(str, 0755) != 0) // 0755 est un mode standard pour les répertoires
        {
            perror("Erreur lors de la création du répertoire");
        }
        return 1;
    }

    return 0;
}
// Permet de gérer les redirections < et >
void gererRedirection(char **arguments)
{
    FILE *fichier; // Pointeur pour ouvrir et rediriger les fichiers
    int i = 0;

    while (arguments[i] != NULL)
    { // On teste tous les arguments pour vérifier si oui ou non, le programme a besoin d'effectuer une redirection

        if (strcmp(arguments[i], ">") == 0)
        {
            if (arguments[i + 1] == NULL)
            {
                fprintf(stderr, "Erreur : aucun fichier spécifié après >.\n");
            } else {
                // Redirection de la sortie standard (stdout) vers le fichier
                fichier = freopen(arguments[i + 1], "w", stdout);
                if (fichier == NULL)
                {
                    // permet d'afficher l'erreur
                    perror("Erreur lors de l'ouverture du fichier pour la redirection de sortie");
                }

                // On supprime alors le symbole > de la liste des arguments
                arguments[i] = NULL;
                break;
            }
        }

        if (strcmp(arguments[i], "<") == 0)
        {
            if (arguments[i + 1] == NULL)
            {
                fprintf(stderr, "Erreur : aucun fichier spécifié après <.\n");
            } else {
                // Redirection de l'entrée standard (stdin) depuis le fichier
                fichier = freopen(arguments[i + 1], "r", stdin);
                if (fichier == NULL)
                {
                    perror("Erreur lors de l'ouverture du fichier pour la redirection d'entrée");
                }

                // On supprime alors le symbole < de la liste des arguments
                arguments[i] = NULL;
                break;
            }
        }

        i++;
    }
}

char* remove_first_and_last(char** arg, int indice) {
    char *str = arg[indice]; // Accéder à la chaîne cible
    size_t len = strlen(str); // Longueur de la chaîne str

    if (str[0] != '\"'){ // s'il n'y a pas de ""
        char *result = malloc(len); // on alloue de la memoire a result
        strncpy(result, str, len); // copie str dans result et definie que resultat fait une taille len
        return result;
    }

    if (len <= 2) {
        // Si la chaîne est trop courte (0, 1 ou 2 caractères), retourner une chaîne vide
        char *result = malloc(1); // Allouer une chaîne vide
        if (!result) {
            perror("malloc");
            return NULL;
        }
        result[0] = '\0';
        return result;
    }
    // Allouer un nouveau buffer pour stocker le résultat
    char *result = malloc(len - 1); // len - 2 pour enlever les caractères, +1 pour le '\0'
    if (!result) {
        perror("malloc");
        return NULL;
    }

    // Copier la partie utile de la chaîne dans le buffer
    strncpy(result, str + 1, len - 2); // Ignorer le premier caractère
    result[len - 2] = '\0';           // Terminer avant le dernier caractère
    return result;
}

// Fonction qui permet de savoir s'il faut lancer la fonction de pipeline
// ou la fonction d'execution de commande standard
void PipelineOrDirect(char* commande, char ***env, History *head, int* term_status){
    // Alloue de la memoir pour argument, la fonction qui contient la commande parser
    char** args = malloc(PARSE_ARG_MAX * sizeof(char*));
    for (int i = 0; i < PARSE_ARG_MAX; i++) {
        args[i] = malloc(ARG_MAX * sizeof(char));
        // On définit une taille volontairement grande comme maximum pour que la probablité qu'une commande  puisse la remplir soit très faible
    }

    // Utiliser parserCommande pour passer de la commande brute à un tableau 2D
    // avec à chaque colonne un element parsé
    parserCommande(commande, args);

    /* Ancien point de Debug
    for (int i = 0; args[i] != NULL && i < 10; i++) {
        //printf("arguments : %s \n", args[i]);
    }
    */

    // Compter le nombre de commandes dans le pipeline
    int pipe_count = 0;

    // Compter le nombre de | dans la commande
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_count++;
        }
    }

    // S'il n'y a pas de pipe alors c'est une commande normale sans pipeline
    if (pipe_count == 0) {
        //executerCommande(args);

        gererCommande(args, env, &head, term_status);
        goto cleanup;
    } else { // sinon on lance le pipeline
        gererPipeline(args);
        goto cleanup; // va directement a cleanup, plus optimiser
    }

cleanup:
    // Libérer de la memoire le tableau args
    for (int i = 0; i < PARSE_ARG_MAX; i++) {
        free(args[i]);
    }
    free(args);
}

// Fonction qui s'occupe des pipelines
void gererPipeline(char** args) {
    /* ancien point de debug
    for (int i = 0; args[i] != NULL && i < 10; i++) {
        //printf("arguments : %s \n", args[i]);
    }
    */

    // Compte le nombre de fonctions dans le pipeline exemple ls -1 | grep '.c' donne 2 fonctions
    int num_commands = 1;
    // Compte le nombre de | dans le pipeline
    int pipe_count = 0;
    // Enregistre la position de chaque | du pipeline
    int pipe_positions[PARSE_ARG_MAX] = {0};
    
    // enregistre la position des | et compte le nombre de fonction et |
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_positions[pipe_count++] = i;
            num_commands++;
        }
    }
    // Initialisation des pipes
    int pipes[PARSE_ARG_MAX][2];
    pid_t pids[PARSE_ARG_MAX];

    for (int i = 0; i < pipe_count; i++) {
        if (pipe(pipes[i]) == -1) { // si pas = -1 alors le pipe s'est bien initialise
            perror("pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Parcours de chaque commande dans le pipeline
    for (int cmd = 0; cmd < num_commands; cmd++) {
        // Allocation pour les arguments de la fonction
        char** cmd_args = malloc(PARSE_ARG_MAX * sizeof(char*));
        int start_idx = (cmd == 0) ? 0 : pipe_positions[cmd-1] + 1; // Début après le pipe précédent
        int end_idx = (cmd == num_commands - 1) ? PARSE_ARG_MAX : pipe_positions[cmd]; // Fin avant le pipe suivant
        int arg_idx = 0; // Index pour remplir les arguments de la commande

        // Extraction des arguments pour la commande actuelle
        for (int i = start_idx; i < end_idx && args[i] != NULL && strcmp(args[i], "|") != 0; i++) {
            cmd_args[arg_idx++] = strdup(args[i]); // Copie de l'argument
        }
        cmd_args[arg_idx] = NULL; // terminer avec NULL

        // Création d'un processus enfant avec fork
        pids[cmd] = fork();
        if (pids[cmd] < 0) { // Vérifie si fork échoue
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pids[cmd] == 0) { // Code exécuté par le processus enfant
            // Redirection de l'entrée standard si ce n'est pas la première commande
            if (cmd > 0) {
                if (dup2(pipes[cmd-1][0], STDIN_FILENO) == -1) { // Redirige l'entrée standard
                    perror("dup2 input failed");
                    exit(EXIT_FAILURE);
                }
            }

            // Redirection de la sortie standard si ce n'est pas la dernière commande
            if (cmd < num_commands - 1) {
                if (dup2(pipes[cmd][1], STDOUT_FILENO) == -1) { // Redirige la sortie standard
                    perror("dup2 output failed");
                    exit(EXIT_FAILURE);
                }
            }

            // Fermeture des descripteurs inutilisés dans l'enfant
            for (int i = 0; i < pipe_count; i++) {
                if (i != cmd - 1) close(pipes[i][0]); // Ferme l'entrée inutilisée
                if (i != cmd) close(pipes[i][1]); // Ferme la sortie inutilisée
            }

            // retire les guillemets de grep s'il en a
            if (strcmp(cmd_args[0], "grep") == 0 && cmd_args[1] != NULL) {
                char* pattern = cmd_args[1];
                size_t len = strlen(pattern);
                if (len >= 2 && pattern[0] == '\'' && pattern[len-1] == '\'') {
                    pattern[len-1] = '\0';
                    memmove(pattern, pattern + 1, len - 1);
                }
            }
             // Exécution de la commande avec execvp
            execvp(cmd_args[0], cmd_args);
            perror("execvp failed"); // Si execvp échoue, affiche une erreur
            exit(EXIT_FAILURE);
        }

        // Code exécuté par le processus parent
        if (cmd > 0) {
            close(pipes[cmd-1][0]); // Ferme l'entrée du pipe précédent
        }
        if (cmd < num_commands - 1) {
            close(pipes[cmd][1]); // Ferme la sortie du pipe actuel
        }

        // Libération de la mémoire pour les arguments de la commande
        for (int i = 0; cmd_args[i] != NULL; i++) {
            free(cmd_args[i]); // Libère chaque argument
        }
        free(cmd_args); // Libère le tableau des arguments
    }

    // Attente de la fin de tous les processus enfants
    for (int i = 0; i < num_commands; i++) {
        int status;
        waitpid(pids[i], &status, 0); // Attend la fin du processus enfant
    }
}



// Gestion spécifique des commandes d'environnement
int gestionEnvironnement(char **arguments, char ***env)
{
    // Vérifie si les arguments sont valides
    if (arguments == NULL || arguments[0] == NULL)
    {
        return 0; // Aucune commande à traiter, retourne 0
    }

    // Commande : set_env <nom> <valeur>
    if (strcmp(arguments[0], "set_env") == 0)
    {
        // Vérifie que le nom et la valeur de la variable sont fournis
        if (arguments[1] != NULL && arguments[2] != NULL)
        {
            addEnv(arguments[1], arguments[2], env); // Ajouter ou mettre à jour la variable
            return 1;
        }
        else
        {
            fprintf(stderr, "Usage : set <nom> <valeur>\n");
            return 1;
        }
    }

    // Commande : unset_env <nom>
    if (strcmp(arguments[0], "unset_env") == 0)
    {
        if (arguments[1] != NULL)
        {
            if (updateEnv(arguments[1], "", env))
            { // Mettre une valeur vide pour "supprimer"
                printf("Variable %s supprimée.\n", arguments[1]);
            }
            else
            {
                printf("Erreur : Variable %s introuvable.\n", arguments[1]);
            }
            return 1;
        }
        else
        {
            fprintf(stderr, "Usage : unset <nom>\n");
            return 1;
        }
    }

    // Commande : printenv
    if (strcmp(arguments[0], "printenv") == 0)
    {
        printEnv(env); // Afficher toutes les variables
        return 1;
    }

    return 0; // La commande ne concerne pas l'environnement
}

void gererCommande(char **arguments, char ***env, History **head, int* term_status)
{
    if (arguments == NULL || arguments[0] == NULL) //Si la commande reçue est vide
    {
        fprintf(stderr, "Erreur : aucune commande spécifiée.\n"); //Renvoyer une erreur
        return;
    }

    if (strcmp(arguments[0], "history") == 0) //Si l'utilisateur a entré la commande history
    {   
        print_history(*head); //Afficher l'historique par la procédure dédiée
        return;
    }

    if (gestionEnvironnement(arguments, env)) //Vérification si l'utilisateur demande une gestion des variables d'environnement
    {
        return; // Commande liée à l'environnement traitée
    }

    int codeCommandeInterne = executerCommandeInterne(arguments); //Enregistrement du code d'exécution d'une commande interne
    if (codeCommandeInterne != 0) //Qui est égal à 0 si la commande n'est pas interne au programme
    {
        if (codeCommandeInterne == -1) { //Si le résultat est -1, alors l'utilisateur a entré exit et demande la sortie du programme
            *term_status = -1; //Modification du statut du terminal pour indiquer l'arrêt
        }
        return; // Commande interne exécutée
    }

    fpos_t pos_out;
    fgetpos(stdout, &pos_out); // Enregistrer la position actuelle du flux de sortie
    int f_out= dup(fileno(stdout)); // Dupliquer le descripteur de fichier associé à stdout

    fpos_t pos_in; //
    fgetpos(stdin, &pos_in); // Enregistrer la position actuelle du flux d'entrée
    int f_in = dup(fileno(stdin)); // Dupliquer le descripteur de fichier associé à stdin

    gererRedirection(arguments);
    executerCommande(arguments);

    fflush(stdout); // Forcer l'écriture de tout contenu dans stdout
    fflush(stdin); // Vider le tampon d'entrée
    dup2(f_out, fileno(stdout)); // Restaurer le flux de stdout
    dup2(f_in, fileno(stdin)); // Restaurer le flux de stdin
    close(f_out); // Fermer le descripteur de stdout
    close(f_in); // Fermer le descripteur de stdin
    clearerr(stdout); // Nettoyer les erreurs
    clearerr(stdin);
    fsetpos(stdout, &pos_out); // Restaurer la position des flux stdout et stdin
    fsetpos(stdin, &pos_in);
}

// Pointeurs globaux pour gérer l'historique
History *head = NULL;
History *tail = NULL;
History *current = NULL;

// Fonction pour ajouter une commande à l'historique
void add_history(History **head, History **tail, const char *command)
{
    History *new_history = (History *)malloc(sizeof(History)); //Allocation dynamique dans la mémoire de la structure de l'historique (chaîne d'une liste chaînée)
    if (!new_history) //Si cette allocation échoue
    {
        fprintf(stderr, "Erreur : allocation mémoire pour l'historique.\n"); //Message d'erreur
        return;
    }

    new_history->line = strdup(command); // Copier la commande
    new_history->index = (*tail) ? (*tail)->index + 1 : 0; //Augmenter l'index
    new_history->next = NULL; //Déclarer la chaîne suivante comme NULL

    if (*tail) //Si la liste chaînée a bien un élément de fin
    {
        (*tail)->next = new_history; //Préciser la nouvelle chaîne comme nouvel élément de fin
        new_history->back = *tail; //Lier l'avant-dernière chaîne avec cette nouvelle chaîne
    }
    else
    {
        *head = new_history; // Sinon il s'agit du premier élément
        new_history->back = NULL;
    }

    *tail = new_history; // Mettre à jour tail
}

// Fonction pour afficher l'historique
void print_history(History *head)
{
    History *temp = head; //Déclaration d'un pointeur de chaîne pour l'historique
    while (temp) //Tant qu'il n'est pas NULL
    {
        printf("%d: %s\n", temp->index, temp->line); //Afficher l'index et le contenu
        temp = temp->next; //Naviguer au suivant
    }
}

void free_history(History **head) { //Procédure pour libérer dans la mémoire l'historique
    History *cursor = *head; //Déclaration d'un pointeur prenant pour élément la tête de l'historique
    History *precursor = NULL; //Déclaration d'un second pointeur pour conserver le précurseur
    while (cursor) { //Tant que le curseur n'est pas égal à NULL
        precursor = cursor; //Affecter l'adresse du curseur actuel à precursor
        cursor = cursor->next; //Naviguer cursor à la chaîne suivante
        free(precursor->line); //Libérer dans la mémoire le curseur initial
        free(precursor);
        precursor = NULL;
    }

}

char ***initEnv(const int max) //Procédure pour initier la matrice des variables d'environnement de l'utilisateur
{
    char ***env = (char ***)malloc(sizeof(char **) * (max + 1)); //Allocation dynamique dans la mémoire
    if (env == NULL) //Renvoyer une erreur si cette allocation échoue
    {
        printf("Error : can't initialize env array");
        exit(1);
    }
    for (int i = 0; i < max + 1; i++) //Pour chaque colonne
    {
        env[i] = (char **)malloc(sizeof(char *) * 2); //Allouer deux lignes
        if (env[i] == NULL)
        {
            printf("Error : can't initialize env array");
            exit(1);
        }
        for (int j = 0; j < 2; j++)
        {
            env[i][j] = (char *)malloc(sizeof(char) * ENV_MAX); //Allouer une chaîne de caractère dans chaque ligne
            if (env[i][j] == NULL)
            {
                printf("Error : can't initialize env array");
                exit(1);
            }
        }
    }
    snprintf(env[0][0], ENV_MAX, "%d", max); //Stocker dans la première ligne de la première colonne le nombre maximum de variables
    snprintf(env[0][1], ENV_MAX, "0"); //Stocker dans la deuxième colonne le nombre actuel de variables
    return env;
}

bool updateEnv(char *name, char *value, char ***env) //Fonction boolean vérifiant si une variable d'environnement existe déjà
{
    for (int i = 1; i < (atoi(env[0][0]) + 1); i++) //En parcourant la matrice des variables d'environnements tout en sachant le nombre de variables d'environnement
    {
        if (strcmp(name, env[i][0]) == 0) //Si le même nom est trouvé
        {
            snprintf(env[i][1], ENV_MAX, "%s", value); //Mettre à jour la valeur
            setenv(name, value, 1); //Mettre à jour dans le système
            return true; //Renvoyer true
        }
    }
    return false; //Sinon renvoyer false
}

void addEnv(char *name, char *value, char ***env) //Procédure pour ajouter une variable d'environnement
{
    if (atoi(env[0][1]) + 1 < atoi(env[0][0])) //Si le nombre de variables est bien inférieure au nombre maximum
    {
        if (!updateEnv(name, value, env)) //Si la variable d'environnement n'existe déjà pas
        {
            setenv(name, value, 0); //L'appliquer au système
            int index = atoi(env[0][1]); //Création d'un index
            index++; //Qui prend la position de la nouvelle variable
            snprintf(env[index][0], ENV_MAX, "%s", name); //Enregistrement du nom
            snprintf(env[index][1], ENV_MAX, "%s", value); //Enregistrement de la valeur
            snprintf(env[0][1], ENV_MAX, "%d", (char)index); //Incrémentation du nombre de variables
        }
    }
    else //S'il n'y a plus de place disponible
    {
        printf("Error ! There is too much custom env, you should have less than %d", atoi(env[0][0]));
    }
}

void printEnv(char ***env) //Procédure pour afficher les variables d'environnement de l'utilisateur
{
    int nb = atoi(env[0][1]); //Récupération du nombre de variables existantes
    for (int i = 1; i <= nb; i++)
    {
        printf("\n%s=%s", env[i][0], env[i][1]); //Affichage de chacune d'entre elle
    }
    printf("\n"); // Juste pour un retour à la ligne
}

void freeEnv(char ***env, int max) { //Procédure pour libérer dans la mémoire la matrice des variables d'environnement de l'utilisateur
    for (int i = 0; i < max + 1; i++) {
        for (int j = 0; j < 2; j++)
        {
            free(env[i][j]);
            env[i][j] = NULL;
        }
        free(env[i]);
        env[i] = NULL;
    }
    free(*env);
    *env = NULL;
    free(env);
    env = NULL;
}