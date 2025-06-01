/**
 * @file parser.h
 * @brief Chargement et accès aux variables d'environnement.
 * @date 2025-05-26
 * @license MIT
 */
#ifndef PARSER_H
#define PARSER_H

/**
 * @brief Charge un fichier .env et stocke les paires clé=valeur.
 * @param[in] filename Chemin vers le fichier .env.
 */
void load_env_file(const char *filename);

/**
 * @brief Récupère la valeur d'une clé d'environnement.
 * @param[in] key Clé recherchée.
 * @return Chaîne de caractères de la valeur, ou NULL.
 */
const char* get_env_value(const char *key);

#endif
