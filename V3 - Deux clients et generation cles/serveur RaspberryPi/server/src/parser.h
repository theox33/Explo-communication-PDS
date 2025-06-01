/**
 * @file parser.h
 * @brief Parser for environment variables from .ini/.env files
 * @version 3.0
 * @author Alexis DEVERCHERE
 * @date 2025
 */

 #ifndef PARSER_H
 #define PARSER_H
 
 /**
  * @brief Load environment variables from a configuration file
  * 
  * Reads a configuration file (typically .env or .ini format) and loads
  * key-value pairs into internal storage for later retrieval.
  * 
  * @param filename Path to the configuration file to load
  * 
  * @note The file format expected is KEY=VALUE with one pair per line
  * @note Empty lines and lines starting with '#' are ignored as comments
  * @note Leading and trailing spaces are automatically trimmed
  */
 void load_env_file(const char *filename);
 
 /**
  * @brief Retrieve the value associated with an environment variable key
  * 
  * Searches the loaded environment variables for the specified key and
  * returns its corresponding value.
  * 
  * @param key The environment variable key to search for
  * @return const char* Pointer to the value string if found, NULL otherwise
  * 
  * @note The returned pointer is valid until the next call to load_env_file()
  * @note Key comparison is case-sensitive
  */
 const char* get_env_value(const char *key);
 
 #endif