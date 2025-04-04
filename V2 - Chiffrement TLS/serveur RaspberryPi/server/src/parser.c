#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENV_VARS 100
#define MAX_LINE_LEN 256

typedef struct {
    char key[128];
    char value[256];
} EnvVar;

static EnvVar env_vars[MAX_ENV_VARS];
static int env_count = 0;

void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

void load_env_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open .env file");
        return;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0'; // split key and value
        char *key = line;
        char *value = eq + 1;

        // Remove leading/trailing spaces
        while (*key == ' ') key++;
        while (*value == ' ') value++;

        strncpy(env_vars[env_count].key, key, sizeof(env_vars[env_count].key) - 1);
        strncpy(env_vars[env_count].value, value, sizeof(env_vars[env_count].value) - 1);
        env_count++;

        if (env_count >= MAX_ENV_VARS) break;
    }

    fclose(file);
}

const char* get_env_value(const char *key) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].key, key) == 0) {
            return env_vars[i].value;
        }
    }
    return NULL;
}
