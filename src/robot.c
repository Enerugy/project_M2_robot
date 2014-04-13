#include "robot.h"


/******************************** Convertion du message en int *******************************/

int tabCharToInt(string message, int *nombre){
	int i;
	puts("nombre");
	// Erreur si le message contient autre chose que des chiffres ou un signe - au début du message
	if (message[0] != '-' && (message[0] < '0' || message[0] > '9')){
		return -1;
	}

	for(i = 0; i < message.size() - 1; i++){
		if (message[message.size() - 1 - i] < '0' || message[message.size() - 1 - i] > '9'){	
			return -1;
		}
		// Chaque caractère récupéré est converti en int et additionné à nombre.
		*nombre += (message[message.size() - 1 - i] - 48)* pow(10,i);
	}
	
	// Si on a un signe - au début du message on multiplie par -1 sinon on ajoute la valeur au nombre.
	if (message[0] == '-'){
		*nombre *= -1;
	}
	else{
		*nombre += (message[0] - 48)* pow(10, message.size() - 3 - 1);	
	}
	printf("nombre : %d\n", *nombre);
	return 0;
}

/**************************************** Prise de mesures **********************************/

int ultrason(int *ptabDistance){
	//threadArg *pArg = (threadArg *) Arg;
	// Prise de mesure
	sem_wait(Arg.verrou_I2C);
	if (i2c_w(ADDR_ULTRASON_3, REGISTRE_W, DETECTION_EN_CM, NULL) != 0){
		return -1;
	}
	sem_post(Arg.verrou_I2C);
	//On attend pour que les capteurs ne se gènent pas pdt une détection
	usleep(WAIT_40_MS);
	// Récupération des données
	sem_wait(Arg.verrou_I2C);
	*(ptabDistance + ULTRASON_3) = (int)(pow(16,2)*(int)i2c_r(ADDR_ULTRASON_3, REGISTRE_RD1, NULL) + (int)i2c_r(ADDR_ULTRASON_3, REGISTRE_RD0, NULL));
	sem_post(Arg.verrou_I2C);
	
	
	sem_wait(Arg.verrou_I2C);
	if (i2c_w(ADDR_ULTRASON_2, REGISTRE_W, DETECTION_EN_CM, NULL) != 0){
		return -1;
	}
	sem_post(Arg.verrou_I2C);
	
	usleep(WAIT_40_MS);
	
	sem_wait(Arg.verrou_I2C);
	*(ptabDistance + ULTRASON_2) = (int)(pow(16,2)*(int)i2c_r(ADDR_ULTRASON_2, REGISTRE_RD1, NULL) + (int)i2c_r(ADDR_ULTRASON_2, REGISTRE_RD0, NULL));
	sem_post(Arg.verrou_I2C);
	
	
	sem_wait(Arg.verrou_I2C);
	if (i2c_w(ADDR_ULTRASON_4, REGISTRE_W, DETECTION_EN_CM, NULL) != 0){
		return -1;
	}
	sem_post(Arg.verrou_I2C);
	
	usleep(WAIT_40_MS);
	
	sem_wait(Arg.verrou_I2C);
	*(ptabDistance + ULTRASON_4) = (int)(pow(16,2)*(int)i2c_r(ADDR_ULTRASON_4, REGISTRE_RD1, NULL) + (int)i2c_r(ADDR_ULTRASON_4, REGISTRE_RD0, NULL));
	sem_post(Arg.verrou_I2C);
	
	
	sem_wait(Arg.verrou_I2C);
	if (i2c_w(ADDR_ULTRASON_1, REGISTRE_W, DETECTION_EN_CM, NULL) != 0){
		return -1;
	}
	sem_post(Arg.verrou_I2C);
	
	usleep(WAIT_40_MS);
	
	sem_wait(Arg.verrou_I2C);
	*(ptabDistance + ULTRASON_1) = (int)(pow(16,2)*(int)i2c_r(ADDR_ULTRASON_1, REGISTRE_RD1, NULL) + (int)i2c_r(ADDR_ULTRASON_1, REGISTRE_RD0, NULL));
	sem_post(Arg.verrou_I2C);
	
	
	sem_wait(Arg.verrou_I2C);
	if (i2c_w(ADDR_ULTRASON_5, REGISTRE_W, DETECTION_EN_CM, NULL) != 0){
		return -1;
	}
	sem_post(Arg.verrou_I2C);
	
	usleep(WAIT_40_MS);
	
	sem_wait(Arg.verrou_I2C);
	*(ptabDistance + ULTRASON_5) = (int)(pow(16,2)*(int)i2c_r(ADDR_ULTRASON_5, REGISTRE_RD1, NULL) + (int)i2c_r(ADDR_ULTRASON_5, REGISTRE_RD0, NULL));
	sem_post(Arg.verrou_I2C);

	return 0;
}

void *magnetometre(void *arg){
	//threadArg *pArg = (threadArg *) Arg;
	int *anglePersonne;
	anglePersonne = (int*)malloc(sizeof(int));
	int angleRobot;
	string message, messageRecu;

	// Prise de mesure :
	sem_wait(Arg.verrou_I2C);
	angleRobot = (int)i2c_r(ADDR_MAGNETOMETRE, CMD_A, 'w', NULL);
	sem_post(Arg.verrou_I2C);
	angleRobot = (int)(angleRobot/10);

	//Envoi un ordre de mesure à la BB de la Personne
	zigbee_w("mesure");

	// Attend de recevoir les données provenant du module Zigbee
	zigbee_r(messageRecu);
	if (strcmp(messageRecu.c_str(), "end_program") == 0){
		// Arrêt du programme
		finProgramme = true;
		pthread_exit(NULL);
	}

	// Convertion du message en int
	tabCharToInt(messageRecu, anglePersonne);

	differenceAngle = abs(angleRobot - *anglePersonne);
	if (differenceAngle > 180){
		differenceAngle = 360 - differenceAngle;
	}

	// Positif pour tourner à gauche
	if (angleRobot < 180){
		if (*anglePersonne > angleRobot && *anglePersonne <= angleRobot + 180){
			differenceAngle *= 1;
		}
		else{
			differenceAngle *= -1;
		}
	}
	else{
		if (*anglePersonne >= angleRobot - 180 && *anglePersonne < angleRobot){
			differenceAngle *= -1;
		}
		else{
			differenceAngle *= 1;
		}
	}
	taskDone = true;
	free(anglePersonne);
}

void *deplacement(void *arg){
	//deplacementArg *Arg = (deplacementArg *) arg;
	
	int distanceObjet[NB_ULTRASON] = {0};
	int *ptabDistance;
	ptabDistance = distanceObjet;
	
	// attente de la synchro synchronisation initiale entre le robot et la personne (positionnement)
	while (flagInit == false){
		if (finProgramme == true){
			pthread_exit(NULL);
		}
	}

	while(1){
		if (ultrason(&ptabDistance) != 0){
			puts("Ultrason failed");
			finProgramme = true;
			pthread_exit(NULL);
		}

		if (ptabDistance[ULTRASON_3] < DISTANCE_TROP_PROCHE)
		{
			//Arret immédiat du robot
			robot.stop();
			robotAvance = false;
		}
		// Il detecte un objet tout droit
		else if (ptabDistance[ULTRASON_3] < DISTANCE_OBJET_FACE_MIN)
		{
			// Il detecte un objet proche a 45° a gauche du robot, et on a un minimum de place sur la droite
			if (ptabDistance[ULTRASON_2] < DISTANCE_OBJET_TRANSVERSE_MIN && ptabDistance[ULTRASON_4] > DISTANCE_TROP_PROCHE)
			{
				// Check pour savoir si on voit le meme mur que le 1er ultrason a detecter, si oui calcul precis de repositionnement
				if (ptabDistance[ULTRASON_1] < DISTANCE_OBJET_COTE_MIN) 
				{
					//Faire tourner le robot de (atan((ptabDistance[ULTRASON_1]+DISTANCE_CENTRE_ULTRASON)/(ptabDistance[ULTRASON_3]+DISTANCE_CENTRE_ULTRASON))*180/PI
					//Sur la droite
					robotTourne = true;
					flagVirage = int(abs(atan((ptabDistance[ULTRASON_1]+DISTANCE_CENTRE_ULTRASON)/(ptabDistance[ULTRASON_3]+DISTANCE_CENTRE_ULTRASON))*180/PI));
					robot.turnRight(60,flagVirage,1);

					robotTourne = false;
					flagVirage = 0;
					robot.goForward(90);
				}
				// Approximation de l'angle pour un cas ou on ira de maniere transversale, mais sans acces au mur du capteur gauche perpendiculaire
				else
				{
					//Faire tourner le robot de 45° sur la droite 
					robotTourne = true;
					flagVirage = 45;
					robot.turnRight(60,45,1);

					robotTourne = false;
					flagVirage = 0;
					robot.goForward(90);

				}
			}
			// Il detecte un objet proche a 45° a droite du robot, et on a un minimum de place sur la gauche
			else if (ptabDistance[ULTRASON_4] < DISTANCE_OBJET_TRANSVERSE_MIN && ptabDistance[ULTRASON_2] > DISTANCE_TROP_PROCHE)
			{
				// Check pour savoir si on voit le meme mur que le 1er ultrason a detecter, si oui calcul precis de repositionnement
				if (ptabDistance[ULTRASON_5] < DISTANCE_OBJET_COTE_MIN)
				{
					//Faire tourner le robot de (atan((ptabDistance[ULTRASON_5]+DISTANCE_CENTRE_ULTRASON)/(ptabDistance[ULTRASON_3]+DISTANCE_CENTRE_ULTRASON))*180/PI
					//Sur la gauche
					robotTourne = true;
					flagVirage = -1*int(abs(atan((ptabDistance[ULTRASON_5]+DISTANCE_CENTRE_ULTRASON)/(ptabDistance[ULTRASON_3]+DISTANCE_CENTRE_ULTRASON))*180/PI));
					robot.turnLeft(60,flagVirage,1);

					robotTourne = false;
					flagVirage = 0;
					robot.goForward(90);
				}
				// Approximation de l'angle pour un cas ou on ira de maniere transversale, mais sans acces au mur du capteur droite perpendiculaire
				else
				{
					//Faire tourner le robot de 45° sur la gauche 
					robotTourne = true;
					flagVirage = -45;
					robot.turnLeft(60,45,1);

					robotTourne = false;
					flagVirage = 0;
					robot.goForward(90);

				}
			}
			// Si il n'a pas detecter d'objet trop proche OU si il a detecter des objets trop proche d'un coté ou de l'autre
			// Alors on vire de l'autre cote
			else if (ptabDistance[ULTRASON_2] < ptabDistance[ULTRASON_4])
			{
				//tourner à droite de 80°
				robotTourne = true;
				flagVirage = 80;
				robot.turnRight(75,80,1);

				robotTourne = false;
				flagVirage = 0;
				robot.goForward(90);
			}
			else if (ptabDistance[ULTRASON_4] < ptabDistance[ULTRASON_2])
			{
				//tourner à gauche de 80°
				robotTourne = true;
				flagVirage = 80;
				robot.turnLeft(75,80,1);

				robotTourne = false;
				flagVirage = 0;
				robot.goForward(90);
			}
			// Pied de table ?
			else if (ptabDistance[ULTRASON_1] < ptabDistance[ULTRASON_5])
			{
				//tourne à droite de 45°
				//wait 1sec
				//tourne à gauche de 45°
				robotTourne = true;
				robot.turnRight(90,45,1);
				robot.goForward(90);
				sleep(1);
				robot.turnLeft(90,45,1);

				robotTourne = false;
				flagVirage = 0;
				robot.goForward(90);
			}
			else if (ptabDistance[ULTRASON_5] < ptabDistance[ULTRASON_1])
			{
				//tourne à gauche de 45°
				//wait 1sec
				//tourne à droite de 45°
				robotTourne = true;
				robot.turnLeft(90,45,1);
				robot.goForward(90);
				sleep(1);
				robot.turnRight(90,45,1);

				robotTourne = false;
				flagVirage = 0;
				robot.goForward(90);
			}
		}
		//On se dirige doucement sur le mur à gauche
		else if (ptabDistance[ULTRASON_2] < (DISTANCE_OBJET_TRANSVERSE_MIN*2) && ptabDistance[ULTRASON_2] < ptabDistance[ULTRASON_1])
		{
			// tourne a droite de 10°
			robotTourne = true;
			flagVirage = 10;
			robot.turnRight(75,10,1);

			robotTourne = false;
			flagVirage = 0;
			robot.goForward(90);
		}
		//On se dirige doucement sur le mur à droite
		else if (ptabDistance[ULTRASON_4] < (DISTANCE_OBJET_TRANSVERSE_MIN*2) && ptabDistance[ULTRASON_4] < ptabDistance[ULTRASON_5])
		{
			// tourne a gauche de 10°
			robotTourne = true;
			flagVirage = 10;
			robot.turnLeft(75,10,1);

			robotTourne = false;
			flagVirage = 0;
			robot.goForward(90);
		}
		//On est parallèle au mur, mais proche du mur de gauche
		else if (ptabDistance[ULTRASON_1] < DISTANCE_MUR_PROCHE)
		{
			// droite a droite de 45° puis instantanement à gauche de 45°
			robotTourne = true;
			flagVirage = 0;
			robot.turnRight(95,45,1);
			robot.turnLeft(95,45,1);

			robotTourne = false;
			robot.goForward(90);
		}
		//On est parallèle au mur, mais proche du mur de droide
		else if (ptabDistance[ULTRASON_5] < DISTANCE_MUR_PROCHE)
		{
			// droite a gauche de 45° puis instantanement à droite de 45°
			robotTourne = true;
			flagVirage = 0;				//Afin d'éviter d'embeter le programme de positionnement
			robot.turnLeft(95,45,1);
			robot.turnRight(95,45,1);

			robotTourne = false;
			robot.goForward(90);
		}
		//Aucun obstacle génant, on va tout droit
		else
		{
			robotTourne = false;
			flagVirage = 0;
		}

		if (finProgramme == true){
			// Fin du thread
			pthread_exit(NULL);
		}
	}
}

void initCamera(){
 	// declenchement de la camera
    cap.set(CV_CAP_PROP_FRAME_WIDTH,SCREEN_WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,SCREEN_HEIGHT);
    Mat capture;

    for (int i = 0; i < 15; i++)
		cap >> capture;	// initialisation cam

}

cameraInformation camera(){
	cameraInformation infoFlag;
	infoFlag.flagDetect = false;
	infoFlag.flagRight = false;
	infoFlag.flagLeft = false;
	infoFlag.rangePers = 0;

	recognize(cap,classifier,kevinRER,1,&infoFlag);// 1 : nombre de reconnaissance demandé	 ps : afin d'initiliser attendre 10 sec avant de se mettre devant la cam  

	return infoFlag;
}

int initPositionnement(){
	pthread_t thread;
	threadArg arg;
	char message[32];
	string messageEnvoi, messageRecu;
	short angleRotationR = 0;

	// Faire l'initialisation de la caméra:
	initCamera();
	
	//Initialisation du magnétomètre:
	sem_wait(Arg.verrou_I2C);
	if (i2c_w(ADDR_MAGNETOMETRE, CALIBRATION_START, NULL) != 0){
		puts("Magnétomètre failed");
		// Arrêt du programme
		finProgramme = true;
		pthread_exit(NULL);
	}
	robot.pivotRight(20);
	sleep(20);
	if (i2c_w(ADDR_MAGNETOMETRE, CALIBRATION_END, NULL) != 0){
		puts("Magnétomètre failed");
		// Arrêt du programme
		finProgramme = true;
		pthread_exit(NULL);
	}
	if (i2c_w(ADDR_MAGNETOMETRE, CMD_A, NULL) != 0){
		puts("Magnétomètre failed");
		// Arrêt du programme
		finProgramme = true;
		pthread_exit(NULL);
	}
	sem_post(Arg.verrou_I2C);

	// Initialisation des variables indiquant l'état dans lequel on se trouve
	robotTourne = false;
	robotAvance = true;
	flagVirage = 0;
	flagStop = true;
	taskDone = false;

	// On tente de trouver la personne en activant la caméra puis en vérifiant que la personne et le robot
	// soient dans la même direction. -> Si la caméra détecte une personne on va lancer la prise de photo.
	do{
		// On regarde dans quelles positions se trouvent la personne par rapport au robot avec les magnéto.
		taskDone = false;
		pthread_create(&thread, NULL, magnetometre, (void *) &arg);
		while(taskDone != true){/* On attend la fin de ta tache */}
		pthread_join(thread, NULL);
		// Si c'est dans le thread magnéto qu'on a reçu l'ordre d'arrêt
		if (finProgramme == true){
			// Fin du thread
			pthread_exit(NULL);
		}

		// Lance la vérif avec le magnétomètre et la caméra sur 360° par palier de 30°:
		while (abs(differenceAngle) > ANGLE_LIMITE){
			sprintf(message, "A: %d", differenceAngle);
			messageEnvoi = message;
			zigbee_w(messageEnvoi);
			// Petite tempo pour laisser le temps à la beagle de la personne de faire des modifs
			usleep(100000);
			// On refait une acquisition des magnéto
			taskDone = false;
			pthread_create(&thread, NULL, magnetometre, NULL);
			while(taskDone != true){/* On attend la fin de ta tache */}
			pthread_join(thread, NULL);
			// Si c'est dans le thread magnéto qu'on a reçu l'ordre d'arrêt
			if (finProgramme == true){
				// Fin du thread
				pthread_exit(NULL);
			}
		}
		// Message indiquant que la personne et le robot sont dans la même direction
		zigbee_w("M: magneto_success");
		// On attend que le son est bien été joué
		zigbee_r(messageRecu);
		if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
			string synchro = "";
			while (strcmp(synchro.c_str(), "synchro_success") != 0){
				zigbee_w("synchronize");
				zigbee_r(synchro);
			}
		}
		else if (strcmp(messageRecu.c_str(), "end_program") == 0){
			// Arrêt du programme
			finProgramme = true;
			pthread_exit(NULL);
		}

		// Mnt que le robot et la personne sont dans la même direction on va pouvoir faire une acquisition
		// pour voir si on trouve quelqu'un.
		cameraInformation = camera();
		if (cameraInformation.flagDetect == false){
			// Si on ne détecte toujours pas la personne on va alors faire pivoter le robot sur lui même de 30°
			// Et ainsi de suite jusqu'à balayer un angle de 180° permettant de vérifier la zone se trouvant à 
			// l'arrière du robot au moment où on s'est rendu compte de la perte de la personne 
			zigbee_w("M: detection_failed");
			// On attend que le son est bien été joué
			zigbee_r(messageRecu);
			if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
				string synchro = "";
				while (strcmp(synchro.c_str(), "synchro_success") != 0){
					zigbee_w("synchronize");
					zigbee_r(synchro);
				}
			}
			else if (strcmp(messageRecu.c_str(), "end_program") == 0){
				// Arrêt du programme
				finProgramme = true;
				pthread_exit(NULL);
			}

			angleRotationR += 30;
			if (angleRotationR >= 360)
			{
				puts("Aucune personne n'a été détectée, arrêt du programme");
				zigbee_w("I: not_found");
				// Arrêt du programme
				finProgramme = true;
				pthread_exit(NULL);
			}
			// Modifier la direction du robot:
			robot.pivotRight(100, 30);
		}
		// Petite tempo pour laisser le temps à la personne de se replacer
		usleep(1500000);
	}while (cameraInformation.flagDetect == false);

	// Si on détecte un visage, on va supposer que c'est la personne qui veut être guidée.
	// On va lancer la prise de photo
	takePicture();

	// Initialisation des ultrasons:
	if (i2c_w(ADDR_ULTRASON_3, REGISTRE_W_RANGE, DETECTION_MAX, NULL) != 0){
		finProgramme = true;
		pthread_exit(NULL);
	}
	if (i2c_w(ADDR_ULTRASON_2, REGISTRE_W_RANGE, DETECTION_MAX, NULL) != 0){
		finProgramme = true;
		pthread_exit(NULL);
	}
	if (i2c_w(ADDR_ULTRASON_4, REGISTRE_W_RANGE, DETECTION_MAX, NULL) != 0){
		finProgramme = true;
		pthread_exit(NULL);
	}
	if (i2c_w(ADDR_ULTRASON_1, REGISTRE_W_RANGE, DETECTION_MAX, NULL) != 0){
		finProgramme = true;
		pthread_exit(NULL);
	}
	if (i2c_w(ADDR_ULTRASON_5, REGISTRE_W_RANGE, DETECTION_MAX, NULL) != 0){
		finProgramme = true;
		pthread_exit(NULL);
	}

	flagInit = true;

	return 0;
}

void *positionnement(void *arg){
	/* Flags: 
				-> flagLeft: true si on détecte que la personne est trop à gauche du robot
				-> flagRight: true si on détecte que la personne est trop à droite du robot
				-> flagDetect: true" si la caméra voit la personne
				-> rangePers: distance entre la personne et le robot
				-> robotTourne: true si le robot tourne
				-> flagStop: true si le robot est arrêté
	*/

	pthread_t thread;
	threadArg arg;
	unsigned char compteur = 0;
	char message[32];
	string messageEnvoi, messageRecu;

	// Initialiser le zigbee
	if (init_ttyUSB(0) == -1){
		puts("Init zigbee failed");
		finProgramme = true;
		pthread_exit(NULL);
	}

	// Attente du signal d'initialisation provenant de la personne.
	do{
		zigbee_r(messageRecu);
		if (strcmp(messageRecu, "I: start_initialisation") != 0){
			puts("Erreur Message d'initialisation");
			zigbee_w("I: not_found");
		}
	}while(strcmp(messageRecu, "I: start_initialisation") != 0);

	//tempo
	
	initPositionnement();

	while(1){
		/* Si le robot va tout droit on vérifie le positionnement de la personne avec la caméra toutes les 2-3 sec */
		/* Tout ce qui change c'est la durée de la tempo en fonction de l'état du robot */
		if (robotAvance == true){
			//Acquisition Cam et Magnéto ssi on va tout droit
			if (robotTourne == false){
				compteur = 0;
				// Acquisition Caméra et en // acquisition magnétomètre ---> lancer les 2 en mm temps et attendre la fin des 2
				taskDone = false;
				// Lancement des 2 taches en //
				pthread_create(&thread, NULL, magnetometre, NULL);
				cameraInformation = camera();
				pthread_join(thread, NULL);
				// Si c'est dans le thread magnéto qu'on a reçu l'ordre d'arrêt
				if (finProgramme == true){
					// Fin du thread
					pthread_exit(NULL);
				}

				// Personne détectée
				if (cameraInformation.flagDetect == true){
					// Vérification de la direction avec les 2 magnétomètres:
					if (abs(differenceAngle) > ANGLE_LIMITE){
						zigbee_w("M: stop");
						// On attend que le son est bien été joué
						zigbee_r(messageRecu);
						if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
							string synchro = "";
							while (strcmp(synchro.c_str(), "synchro_success") != 0){
								zigbee_w("synchronize");
								zigbee_r(synchro);
							}
						}
						else if (strcmp(messageRecu.c_str(), "end_program") == 0){
							// Arrêt du programme
							finProgramme = true;
							pthread_exit(NULL);
						}
						
						flagStop = true;
						while (abs(differenceAngle) > ANGLE_LIMITE){
							sprintf(message, "A: %d", differenceAngle);
							messageEnvoi = message;
							zigbee_w(messageEnvoi);
							// Petite tempo pour laisser le temps à la personne de se déplacer, A voir.
							usleep(100000);
							// On refait une acquisition des magnéto
							taskDone = false;
							pthread_create(&thread, NULL, magnetometre, NULL);
							while(taskDone != true){/* On attend la fin de ta tache */}
							pthread_join(thread, NULL);
							// Si c'est dans le thread magnéto qu'on a reçu l'ordre d'arrêt
							if (finProgramme == true){
								// Fin du thread
								pthread_exit(NULL);
							}
						}
						// Message indiquant que la personne et le robot sont dans la même direction
						zigbee_w("M: magneto_success");
						// On attend que le son est bien été joué
						zigbee_r(messageRecu);
						if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
							string synchro = "";
							while (strcmp(synchro.c_str(), "synchro_success") != 0){
								zigbee_w("synchronize");
								zigbee_r(synchro);
							}
						}
						else if (strcmp(messageRecu.c_str(), "end_program") == 0){
							// Arrêt du programme
							finProgramme = true;
							pthread_exit(NULL);
						}
					}
					else{
						if (cameraInformation.flagLeft == true){
							// La personne est trop à gauche
							// Les valeurs des magnéto sont bonnes, on a juste à dire à la personne de faire un pas
							// Dire à la personne de faire un pas à droite
							zigbee_w("M: right");
							// On attend que le son est bien été joué
							zigbee_r(messageRecu);
							if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
								string synchro = "";
								while (strcmp(synchro.c_str(), "synchro_success") != 0){
									zigbee_w("synchronize");
									zigbee_r(synchro);
								}
							}
							else if (strcmp(messageRecu.c_str(), "end_program") == 0){
								// Arrêt du programme
								finProgramme = true;
								pthread_exit(NULL);
							}
						}
						else if (cameraInformation.flagRight == true){
							// La personne est trop à droite
							// Les valeurs des magnéto sont bonnes, on a juste à dire à la personne de faire un pas
							// Dire à la personne de faire un pas à droite
							zigbee_w("M: left");
							// On attend que le son est bien été joué
							zigbee_r(messageRecu);
							if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
								string synchro = "";
								while (strcmp(synchro.c_str(), "synchro_success") != 0){
									zigbee_w("synchronize");
									zigbee_r(synchro);
								}
							}
							else if (strcmp(messageRecu.c_str(), "end_program") == 0){
								// Arrêt du programme
								finProgramme = true;
								pthread_exit(NULL);
							}
						}
						else{
							// La personne est bien au centre de la caméra
							// La personne se déplace parfaitement
							// On peut de nouveau faire avancer le robot
							if (flagStop == true){
								zigbee_w("M: go_straight");
								// On attend que le son est bien été joué
								zigbee_r(messageRecu);
								if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
									string synchro = "";
									while (strcmp(synchro.c_str(), "synchro_success") != 0){
										zigbee_w("synchronize");
										zigbee_r(synchro);
									}
								}
								else if (strcmp(messageRecu.c_str(), "end_program") == 0){
									// Arrêt du programme
									finProgramme = true;
									pthread_exit(NULL);
								}
							}
							flagStop = false;
						}
					}
					
					// Si rangePers est trop petite ou trop élevé on modifiera la vitesse du robot
					if (flagStop == false && cameraInformation.rangePers > RANGE_MAX)

					}
					else if (flagStop == false && cameraInformation.rangePers < RANGE_MIN){

					}

					//Si on roule
					if (flagStop == false)
					{
						// Le robot va tout droit et avance, tempo d'environ 2sec.
						sleep(2);
					}	
				}
				else{
					// Si la personne n'a pas été détectée
					zigbee_w("M: lost");
					// On attend que le son est bien été joué
					zigbee_r(messageRecu);
					if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
						string synchro = "";
						while (strcmp(synchro.c_str(), "synchro_success") != 0){
							zigbee_w("synchronize");
							zigbee_r(synchro);
						}
					}
					else if (strcmp(messageRecu.c_str(), "end_program") == 0){
						// Arrêt du programme
						finProgramme = true;
						pthread_exit(NULL);
					}
					// On stop les moteurs du robot
					flagStop = true;
					short angleRotationR = 0;
					while(cameraInformation.flagDetect == false && angleRotationR < 180){
						// Lance la vérif avec le magnétomètre et la caméra en 6 étapes avec interval de 30°:
						while (abs(differenceAngle) > ANGLE_LIMITE){
							sprintf(message, "A: %d", differenceAngle);
							messageEnvoi = message;
							zigbee_w(messageEnvoi);
							// Petite tempo pour laisser le temps à la beagle de la personne de faire des modifs
							usleep(100000);
							// On refait une acquisition des magnéto
							taskDone = false;
							pthread_create(&thread, NULL, magnetometre, NULL);
							while(taskDone != true){/* On attend la fin de la tache */}
							pthread_join(thread, NULL);
							// Si c'est dans le thread magnéto qu'on a reçu l'ordre d'arrêt
							if (finProgramme == true){
								// Fin du thread
								pthread_exit(NULL);
							}
						}
						// Message indiquant que la personne et le robot sont dans la même direction
						zigbee_w("M: magneto_success");
						// On attend que le son est bien été joué
						zigbee_r(messageRecu);
						if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
							string synchro = "";
							while (strcmp(synchro.c_str(), "synchro_success") != 0){
								zigbee_w("synchronize");
								zigbee_r(synchro);
							}
						}
						else if (strcmp(messageRecu.c_str(), "end_program") == 0){
							// Arrêt du programme
							finProgramme = true;
							pthread_exit(NULL);
						}

						// Mnt que le robot et la personne sont dans la même direction on va pouvoir refaire une acquisition
						cameraInformation = camera();
						if (cameraInformation.flagDetect == false){
							// Si on ne détecte toujours pas la personne on va alors faire pivoter le robot sur lui même de 30°
							// Et ainsi de suite jusqu'à balayer un angle de 180° permettant de vérifier la zone se trouvant à 
							// l'arrière du robot au moment où on s'est rendu compte de la perte de la personne 
							zigbee_w("M: detection_failed");
							// On attend que le son est bien été joué
							zigbee_r(messageRecu);
							if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
								string synchro = "";
								while (strcmp(synchro.c_str(), "synchro_success") != 0){
									zigbee_w("synchronize");
									zigbee_r(synchro);
								}
							}
							else if (strcmp(messageRecu.c_str(), "end_program") == 0){
								// Arrêt du programme
								finProgramme = true;
								pthread_exit(NULL);
							}

							angleRotationR += 30;
							if (angleRotationR != 120){
								robot.pivotRight(100, 30);
							}
							else{
								robot.pivotLeft(100, 180);
							}
						}
						// Petite tempo pour laisser le temps à la personne de se replacer
						usleep(1500000);
						// Remettre à jour les positions pour le prochain test
						taskDone = false;
						pthread_create(&thread, NULL, magnetometre, NULL);
						while(taskDone != true ){/* On attend la fin de ta tache */}
						pthread_join(thread, NULL);
						// Si c'est dans le thread magnéto qu'on a reçu l'ordre d'arrêt
						if (finProgramme == true){
							// Fin du thread
							pthread_exit(NULL);
						}
					}
					if (angleRotationR >= 180){
						// On a trouvé personne
						zigbee_w("I: recuperation_failed");
						// Arrêter le programme
						finProgramme = true;
						pthread_exit(NULL);
					}
					// Envoi message à la personne pour lui indiquer
					zigbee_w("M: detection_success");
					// On attend que le son est bien été joué
					zigbee_r(messageRecu);
					if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
						string synchro = "";
						while (strcmp(synchro.c_str(), "synchro_success") != 0){
							zigbee_w("synchronize");
							zigbee_r(synchro);
						}
					}
					else if (strcmp(messageRecu.c_str(), "end_program") == 0){
						// Arrêt du programme
						finProgramme = true;
						pthread_exit(NULL);
					}
				}
			}
			else{
				// Le robot tourne
				if (compteur == 0){
					// Si c'est la première fois que l'on passe par là, on indique à la personne 
					// de combien de degré le robot tourne
					if (flagVirage > 0){
						sprintf(message, "V: robot_turn_right @%d", abs(flagVirage));
					}
					else{
						sprintf(message, "V: robot_turn_left @%d", abs(flagVirage));
					}
					messageEnvoi = message;
					zigbee_w(messageEnvoi);
					// On attend que le son est bien été joué
					zigbee_r(messageRecu);
					if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
						string synchro = "";
						while (strcmp(synchro.c_str(), "synchro_success") != 0){
							zigbee_w("synchronize");
							zigbee_r(synchro);
						}
					}
					else if (strcmp(messageRecu.c_str(), "end_program") == 0){
						// Arrêt du programme
						finProgramme = true;
						pthread_exit(NULL);
					}
				}
				compteur++;
				sleep(1);
			}
		}
		else{
			// Le robot est amené à s'arrêter à cause d'obstacles, indiquer à la personne la marche à suivre
			// A faire
			zigbee_w("M: too_many_obstacles");
			// On attend que le son est bien été joué
			zigbee_r(messageRecu);
			if (strcmp(messageRecu.c_str(), "need_synchro") == 0){
				string synchro = "";
				while (strcmp(synchro.c_str(), "synchro_success") != 0){
					zigbee_w("synchronize");
					zigbee_r(synchro);
				}
			}
			
			// Arrêt du programme
			finProgramme = true;
			pthread_exit(NULL);
		}
	}
}

int main(int argc, char **argv){
	int i;
	pthread_t thread[NB_THREAD];
	threadArg argument[NB_THREAD];

	// Initaliser le robot et checker si les pwm sont bien initalisées
	Robot robot;

	sem_t *verrou_I2C;
	sem_init(&verrou, 0, 1);
	Arg.verrou_I2C = &verrou_I2C;

	finProgramme = false;

	for(i = 0; i < NB_THREAD; i++){
		switch (i){
			case 0 :
				pthread_create(&thread[i], NULL, deplacement, NULL); 
				break;
			case 1 :
				pthread_create(&thread[i], NULL, positionnement, NULL);
				break;
		}
	}

	for(i = 0; i < NB_THREAD; i++){
		pthread_join(thread[i], NULL);
	}
	puts("Fin du programme");

	return 0;
}