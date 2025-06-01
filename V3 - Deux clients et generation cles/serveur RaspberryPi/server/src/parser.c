/**
 * @file parser.c
 * @brief Implementation of environment variable parser
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 /** @brief Maximum number of environment variables that can be stored */
 #define MAX_ENV_VARS 100
 
 /** @brief Maximum length of a line in the configuration file */
 #define MAX_LINE_LEN 256
 
 /**
  * @brief Structure to store a key-value pair for environment variables
  */
 typedef struct {
     char key[128];    /**< Environment variable key */
     char value[256];  /**< Environment variable value */
 } EnvVar;
 
 /** @brief Static array to store all loaded environment variables */
 static EnvVar env_vars[MAX_ENV_VARS];
 
 /** @brief Current number of loaded environment variables */
 static int env_count = 0;
 
 /**
  * @brief Remove trailing newline and carriage return characters from a string
  * 
  * This function modifies the input string in place by replacing trailing
  * newline (\n) and carriage return (\r) characters with null terminators.
  * 
  * @param str String to trim (modified in place)
  */
 void trim_newline(char *str) {
     size_t len = strlen(str);
     while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
         str[len - 1] = '\0';
         len--;
     }
 }
 
 /**
  * @brief Load environment variables from a configuration file
  * 
  * Reads a configuration file line by line and parses KEY=VALUE pairs.
  * Empty lines and lines starting with '#' are treated as comments and ignored.
  * Leading and trailing spaces around keys and values are automatically trimmed.
  * 
  * @param filename Path to the configuration file to load
  * 
  * @note If the file cannot be opened, an error message is printed to stderr
  * @note The function stops loading if MAX_ENV_VARS is reached
  * @note Previously loaded variables are not cleared before loading new ones
  */
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
 
 /**
  * @brief Retrieve the value associated with an environment variable key
  * 
  * Performs a linear search through the loaded environment variables to find
  * a matching key and returns its corresponding value.
  * 
  * @param key The environment variable key to search for
  * @return const char* Pointer to the value string if found, NULL if not found
  * 
  * @note Key comparison is case-sensitive using strcmp()
  * @note The returned pointer points to internal storage and remains valid
  *       until the next call to load_env_file()
  */
 const char* get_env_value(const char *key) {
     for (int i = 0; i < env_count; i++) {
         if (strcmp(env_vars[i].key, key) == 0) {
             return env_vars[i].value;
         }
     }
     return NULL;
 }