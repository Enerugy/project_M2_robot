/**
    @file zigbee.h
    @author ise2
    @version 1.0
    @date 8 mars 2014
 
    @brief Fonctions permettant de configurer une interface usb ainsi que d'envoyer et recevoir des données via zigbee
*/

//****************************************** Include *******************************************
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h> 
#include <string.h>
#include <iostream>

using namespace std;

/**
    @brief	Permet d'initialiser l'interface USB désirée afin de communiquer via la liaison zigbee.
    
    @details	Initialise l'interface USB que l'on donne en paramètre. On règle alors le baudRate, le contrôle de flux etc...

    @param USB 	Int permet d'indiquer le numéro de l'interface USB que l'on veut configurer
	
    @return 0: la configuration s'est bien passé\n
            -1: erreur dans la configuration\n
 */
int init_ttyUSB(int USB);

/**
    @brief	Permet d'envoyer un message via la liaison zigbee.
    
    @details	Envoie le message donné en paramètre sur l'interface USB que l'on a précédement configurée.

    @param message 	Variable contenant le message à envoyer

    @return 0: pas d'erreur \n
            -1: erreur d'ouverture ou d'écriture.\n
 */
int zigbee_w(string message);

/**
    @brief	Permet de recevoir un message via la liaison zigbee.
    
    @details	Reçoit toutes données arrivant sur l'interface USB que l'on a précédement configurée jusqu'à ce que l'on reçoivent un '\0'.

    @param buffer   string contenant le message reçu
    
    @return 0: pas d'erreur.\n
 */
int zigbee_r(string &buffer);
