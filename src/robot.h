#ifndef H_ROBOT_H
#define H_ROBOT_H 

/**
    @file robot.h
    @author ise2
    @version 1.0
    @date 18 mars 2014
 
    @brief	Fonctions permettant au robot de se déplacer en fonction de son environnement ainsi que de 
    		surveiller la personne et de lui indiquer la marche à suivre via zigbee.
 
    @details 	Programmation de l'intelligence du robot lui permettant de se déplacer en détectant les obstacles
    			environnant ainsi que de vérifier la présence de la personne au travers d'une caméra mais aussi
    			de vérifier sa position grâce à deux magnétomètres. Le robot indique également la marche à suivre
    			à la personne après analyse des différents capteurs, en envoyant des données à l'autre carte via zigbee.\n
    		 	- transfert de données via zigbee vers l'autre carte.\n
    		 	- prise de mesure avec le magnétomètre.\n
    		 	- détection d'obstacles.\n
    		 	- déplacement du robot.\n
    		 	- détection de la personne avec une caméra.\n
    		 	- repositionnement de la personne par rapport au robot en fonction des données récupérées.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <assert.h>
#include "i2c_vf.h"
#include "zigbee.h"
#include "cameraBB.hpp"

//********************************************* Define **********************************************

//**************************************** Magnétonmètre *******************************************
/**
    @def ADDR_MAGNETOMETRE
    Adresse du magnétomètre i2c
*/
#define ADDR_MAGNETOMETRE 0x21

/**
    @def CMD_A
    Commande permettant d'indiquer au magnétomètre de faire une acquisition
*/
#define CMD_A 0x41

/**
    @def CALIBRATION_START
    Commande permettant d'indiquer au magnétomètre de faire une acquisition
*/
#define CALIBRATION_START 0x43

/**
    @def CALIBRATION_END
    Commande permettant d'indiquer au magnétomètre de faire une acquisition
*/
#define CALIBRATION_END 0x45

//************************************* Adresses des capteurs ***************************************

/**
    @def ADDR_ULTRASON_1
    Adresse i2c du capteur ultrason 1
*/
#define ADDR_ULTRASON_1 0x71

/**
    @def ADDR_ULTRASON_2
    Adresse i2c du capteur ultrason 2
*/
#define ADDR_ULTRASON_2 0x72

/**
    @def ADDR_ULTRASON_3
    Adresse i2c du capteur ultrason 3
*/
#define ADDR_ULTRASON_3 0x73

/**
    @def ADDR_ULTRASON_4
    Adresse i2c du capteur ultrason 4
*/
#define ADDR_ULTRASON_4 0x74

/**
    @def ADDR_ULTRASON_5
    Adresse i2c du capteur ultrason 5
*/
#define ADDR_ULTRASON_5 0x75

/************************************* Valeurs utiles à la detection ********************************/
/**
    @def NB_ULTRASON
    On a 5 capteurs ultrason
*/
#define NB_ULTRASON 5

/**
    @def ULTRASON_1
    Indice de l'ultrason 1
*/
#define ULTRASON_1 0

/**
    @def ULTRASON_2
    Indice de l'ultrason 2
*/
#define ULTRASON_2 1

/**
    @def ULTRASON_3
    Indice de l'ultrason 3
*/
#define ULTRASON_3 2

/**
    @def ULTRASON_4
    Indice de l'ultrason 4
*/
#define ULTRASON_4 3

/**
    @def ULTRASON_5
    Indice de l'ultrason 5
*/
#define ULTRASON_5 4

/**
    @def DETECTION_EN_CM
    Commande à envoyer aux ultrasons pour qu'ils renvoient la distance en cm
*/
#define DETECTION_EN_CM 0x51 

/**
    @def WAIT_40_MS
    Temps d'attente entre deux prises de mesures avec les ultrasons
*/
#define WAIT_40_MS 40000

/**
    @def DECTION_MAX
    Valeur à envoyer aux capteurs ultrason pour leur donner une distance de détection max de 6 mètres (3*2 / 0.043 = 139.5)
*/
#define DETECTION_MAX 139


/************************************* Registres des capteurs ***************************************/
/**
    @def REGISTRE_W
    Registre dans lequel on vient écrire pour configurer les ultrasons
*/
#define REGISTRE_W 0x00

/**
    @def REGISTRE_W_RANGE
    Registre dans lequel on vient configurer la distance maximale de détection
*/
#define REGISTRE_W_RANGE 0x02

/**
    @def REGISTRE_RD1
    Registre contenant le MSB de la distance mesurée
*/
#define REGISTRE_RD1 0x02 

/**
    @def REGISTRE_RD0
    Registre contenant le LSB de la distance mesurée
*/
#define REGISTRE_RD0 0x03

//***************************************** Positionnement *******************************************
/**
    @def DISTANCE_CENTRE_ULTRASON
    Distance en cm qui sépare le capteur ultrason au centre du robot
*/
#define DISTANCE_CENTRE_ULTRASON 9

/**
    @def DISTANCE_OBJET_FACE_MIN
    Distance en cm à partir de laquelle le robot va effectuer une manoeuvre d'évitement lorsqu'il détecte un objet de face
*/
#define DISTANCE_OBJET_FACE_MIN 200

/**
    @def DISTANCE_OBJET_TRANSVERSE_MIN
  	Distance en cm à partir de laquelle le robot va effectuer une manoeuvre d'évitement lorsque les capteurs 2 et 4 
  	détectent un objet
*/
#define DISTANCE_OBJET_TRANSVERSE_MIN 80

/**
    @def DISTANCE_OBJET_COTE_MIN
    Distance en cm à partir de laquelle le robot va effectuer une manoeuvre d'évitement lorsqu'il 
  	détecte un objet sur sa droite ou sa gauche
*/
#define DISTANCE_OBJET_COTE_MIN 200

/**
    @def DISTANCE_MUR_PROCHE
    Distance d'un mur
*/
#define DISTANCE_MUR_PROCHE 80

/**
    @def DISTANCE_TROP_PROCHE
    Distance considérée comme étant critique
*/
#define DISTANCE_TROP_PROCHE 50

/**
    @def PI
    valeur de pi
*/
#define PI 3.14159265

/***************************************** Define Thread ********************************************/
/**
    @def NB_THREAD
    Nombre de thread à lancer depuis le main (positionnement et deplacement)
*/
#define NB_THREAD 2
/**
    @def ANGLE_LIMITE
    Ecart en dégré à partir duquel on va dire que la personne et le robot ne se déplacent plus dans la même direction
*/
#define ANGLE_LIMITE 25

//************************************* Variables Globales ******************************************

/**
	@var static bool flagInit
	Booléen permettant de synchroniser les 2 threads (positionnement et deplacement)
*/
static bool flagInit = false;

/**
	@var static bool robotTourne
	Booléen indiquant si le robot tourne ou non
*/
static bool robotTourne = false;

/**
	@var static bool robotAvance
	Booléen indiquant si le robot peut avancer ou s'il est bloqué à cause d'un trop grand nombre d'obstacles
*/
static bool robotAvance = true;

/**
	@var static signed short flagVirage
	Variable indiquant de combien de degrés le robot tourne
*/
static signed short flagVirage;

/**
	@var static bool flagStop
	Booléen indiquant que le robot et la personne doivent s'arrêter
*/
static bool flagStop = false;

/**
	@var static int differenceAngle
	Variable indiquant l'écart de direction en degrés entre le robot et la personne
*/
static int differenceAngle;

/**
	@var static bool taskDone
	Booléen indiquant si le thread du magnétomètre est terminé
*/
static bool taskDone;

/**
	@var static bool finProgramme
	Booléen indiquant si le programme doit s'arrêter
*/
static bool finProgramme;

/**
    Structure utilisée pour passer des données aux threads
*/
typedef struct {
	sem_t *verrou_I2C;
}threadArg Arg;

/**
    Structure contenant toutes les données que retourne la caméra permettant de savoir où se trouve la personne
*/
typedef struct{
	bool flagLeft;
	bool flagRight;
	bool flagDetect;
	int rangePers;
}infoCam cameraInformation;


//****************************************** Fonctions *********************************************

/**
    @brief	Permet de faire la convertion d'un char* en un int.
    
    @details	Permet de récupérer une valeur reçue via la liaison zigbee qui est contenu dans un char*.

    @param message 	string contenant le message à convertir en int
    @param nombre	Paramètre permettant de récupérer le résultat de la conversion
    @return 0: pas d'erreur \n
            -1: erreur, le message n'est pas un entier.\n
 */
int tabCharToInt(string message, int *nombre);

/**
    @brief	Fonction permettant de faire l'acquisition sur les 5 capteurs ultrasons
    
    @details	Permet de récupérer la distance séparant le robot d'un obstacle suivant 5 directions

    @param ptabDistance Paramètre permettant de récupérer les 5 distances mesurées par les capteurs.
    @return 0: pas d'erreur \n
            -1: erreur\n
 */
int ultrason(int *ptabDistance);

/**
    @brief	Thread permettant d'effectuer une acquisition sur le magnétomètre du robot et sur le magnétomètre de la personne
    		en envoyant un ordre de mesure via zigbee.
    
    @details	Ce thread permet de faire une acquisition sur le magnétomètre de la personne et envoi un ordre à la beaglebone
    			de la personne afin qu'elle fasse elle aussi une acquisition sur son magnétomètre. Pendant ce temps le thread
    			va attendre de recevoir le résultat de la mesure de la personne. On va ensuite faire la différence de ces deux
    			mesures afin de savoir si le robot et la personne se déplacent dans la même direction.

    @param arg 	Paramètre permettant de passer à un thread des données qui lui sont propres.
 */
void *magnetometre(void *arg);

/**
    @brief	Thread permettant au robot de se déplacer en fonction des données des capteurs à ultrasons.
    
    @details	Le thread lance une acquisition des capteurs à ultrason et il va ensuite analyser les résultats des capteurs
                et en déduire la trajectoire à suivre.

    @param arg 	Paramètre permettant de passer à un thread des données qui lui sont propres.
 */
void *deplacement(void *arg);

/**
    @brief	Fonction permettant l'initialisation des différents capteurs et de positionner le robot et la personne
            dans le même axe.
    
    @details	Fonction permettant de positionner le robot et la personne dans le même axe ainsi que d'initialiser:\n
                - Magnétomètre\n
                - Capteurs ultrasons\n
                - Caméra et prise de photo\n
                - Variables globales\n

    @return 0: pas d'erreur \n
 */
int initPositionnement();

/**
    @brief	Thread permettant de vérifier que la personne suit bien le robot.
    
    @details    Si la personne suit bien le robot la personne sera uniquement averti des manoeuvres faites par le robot
                sinon on va indiquer à la personne en transmettant des messages par zigbee la marche à suivre afin que
                le robot et la personne puisse se resynchroniser.
    
    @param arg  Paramètre permettant de passer à un thread des données qui lui sont propres.
 */
void *positionnement(void *arg);

#endif