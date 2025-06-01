#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENV_VARS 100      // Nombre maximal de variables d'environnement à stocker
#define MAX_LINE_LEN 256      // Longueur maximale d'une ligne dans le fichier .ini

// Structure pour stocker une variable d'environnement (clé/valeur)
typedef struct {
    char key[128];            // Clé de la variable
    char value[256];          // Valeur de la variable
} EnvVar;

static EnvVar env_vars[MAX_ENV_VARS]; // Tableau statique pour stocker les variables
static int env_count = 0;             // Compteur du nombre de variables chargées

/**
 * @brief Supprime les retours à la ligne (\n, \r) à la fin d'une chaîne.
 * @param[in,out] str Chaîne à nettoyer.
 */
void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

/**
 * @brief Charge les variables d'environnement depuis un fichier .ini.
 *        Chaque ligne doit être au format clé=valeur.
 *        Les lignes vides et les commentaires (#) sont ignorés.
 * @param[in] filename Chemin du fichier à charger.
 */
void load_env_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open .ini file");
        return;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);

        // Ignore les lignes vides et les commentaires
        if (line[0] == '\0' || line[0] == '#') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue; // Ignore si pas de '='

        *eq = '\0'; // Coupe la ligne en clé et valeur
        char *key = line;
        char *value = eq + 1;

        // Supprime les espaces en début de clé et de valeur
        while (*key == ' ') key++;
        while (*value == ' ') value++;

        // Copie la clé et la valeur dans le tableau
        strncpy(env_vars[env_count].key, key, sizeof(env_vars[env_count].key) - 1);
        strncpy(env_vars[env_count].value, value, sizeof(env_vars[env_count].value) - 1);
        env_count++;

        if (env_count >= MAX_ENV_VARS) break; // Limite atteinte
    }

    fclose(file);
}

/**
 * @brief Récupère la valeur associée à une clé donnée.
 * @param[in] key Clé recherchée.
 * @return Pointeur vers la valeur trouvée, ou NULL si la clé n'existe pas.
 */
const char* get_env_value(const char *key) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].key, key) == 0) {
            return env_vars[i].value;
        }
    }
    return NULL;
}