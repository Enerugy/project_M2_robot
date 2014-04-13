#ifndef I2C_H
#define I2C_H

/**
    @file i2c.h
    @author ise2
    @version 1.0
    @date 8 mars 2014
 
    @brief Fonctions permettant la communication via une liaison série i2c.
*/

//******************************************* Include ****************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

//******************************************* Define *****************************************
/**
	@def TAILLE_BUFFER
	Taille du buffer
*/
#define TAILLE_BUFFER 2

/**
    @brief	Permet de lire une donnée située dans un composant par i2c.
    
    @details	Permet à l'utilisateur de lire des données via un bus i2c.\n
    			Il n'y a pas un nombre de paramètre prédéfini puisque l'on a 
    			parfois besoin de plus de 2 paramètres pour pouvoir récupérer une donnée.\n
				Par défaut on ne lit qu'un seul octet mais il est possible d'en lire 2 si 
				l'on rajoute un paramètre valant 'w' (word).\n
				Il faut toujours rajouter un dernier paramètre NULL pour prendre en compte le bon nombre de paramètre.

    @param adresse 	Indique l'adresse du composant i2c.
	@param registre Indique le registre dans lequel on souhaite lire la donnée ou la commande requise pour lire sur le composant.
    
    @return valeurLue: contient la valeur que l'on voulait obtenir\n
 */
int i2c_r(char adresse, char registre, ...);

/**
    @brief	Permet d'écrire, commander le composant i2c.
    
    @details	Permet à l'utilisateur d'écrire des données via un bus i2c.\n
				On peut ajouter le nombre de paramètre que l'on veut pour pouvoir communiquer avec le composant i2c.\n
				Il faut toujours rajouter un dernier paramètre NULL pour prendre en compte le bon nombre de paramètre.

    @param adresse	indique l'adresse du composant i2c.
	@param registre Indique le registre dans lequel on souhaite lire la donnée ou la commande requise pour lire sur le composant.

    @return 0: l'écriture s'est bien passé\n
            -1: erreur d'écriture\n
 */
int i2c_w(char adresse, char registre, ...);

#endif