#include "i2c.h"

//************************************* Variables Globales ***********************************
/**
	@var static char buffer
	Buffer contenant les différents paramètres dont on a besoin pour communiquer avec un composant.
*/
static char buffer[TAILLE_BUFFER] = {0};
/**
	@var static char file
	Chemin de l'interface utilisée pour la communication i2c.
*/
static char file[32] = "/dev/i2c-3";

int i2c_r(char adresse, char registre, ...){
	int fd, i, nbParametreOptionnel = 0;
	int valeurLue = 0;
	
	// Permet d'avoir un nombre de paramètre indéfini.
	va_list ap;
	// On récupère le paramètre à partir duquel on aura des paramètres optionnels.
	va_start(ap, registre);
	// On récupère un paramètre optionnel.
	char parametre = (char)va_arg(ap, int);

	// Ouverture du fichier
	if((fd = open(file, O_RDWR)) < 0){
		perror("open failed");
	}
	
	// On vérifie qu'il y a bien un composant i2c à cette adresse.
	if(ioctl(fd, I2C_SLAVE, adresse) < 0){
		perror("ioctl error");
	}
	

	buffer[0] = registre;
	if(write(fd, buffer, 1) != 1 ){ // On écrit 1 octet, la valeur du registre
		perror("write failed");
	}

	// On test si le paramètre optionnel vaut NULL
	while(parametre != '\0'){
		// On obtient le paramètre actuel et on le compare à 'w' qui signifie que l'on veut lire 2 octets
		if(parametre == 'w'){nbParametreOptionnel++;}
		parametre = (char)va_arg(ap, int);
	}
	// S'il n'y a pas de paramètre optionnel on ne lit qu'un octet
	//On lit un registre et on met la valeur lue dans le buffer
	if(read(fd, buffer, nbParametreOptionnel + 1) != (nbParametreOptionnel + 1)){
		perror("read failed");
	}
	// On calcule le résultat en fonction des données récupérées.
	for(i = 0; i < (nbParametreOptionnel + 1); i++){
		valeurLue = valeurLue + (int)((int)buffer[i]*pow(16, 2*(nbParametreOptionnel - i)));
	}

	/* Lecture ok */
	va_end(ap);
	close(fd);

	return valeurLue;
}

int i2c_w(char adresse, char registre, ...){
	int fd, nbParametreOptionnel = 0;
	va_list ap;
	va_start(ap, registre);
	char parametre = (char)va_arg(ap, int);

	if((fd = open(file, O_RDWR)) < 0){
		perror("open failed");
		return -1;
	}
	
	if(ioctl(fd, I2C_SLAVE, adresse) < 0){
		perror("ioctl error");
		return -1;
	}
	
	buffer[nbParametreOptionnel] = registre;
	
	while(parametre != '\0'){ //On récupère les paramètres optionnelles
		nbParametreOptionnel++;
		buffer[nbParametreOptionnel] = parametre;
		parametre = (char)va_arg(ap, int);
	}
	if(write(fd, buffer, nbParametreOptionnel + 1) != (nbParametreOptionnel + 1)){ // On écrit 2 octets
		perror("write failed");
		return -1;
	}
	va_end(ap);
	close(fd);
	/* Écriture ok */
	return 0;
}