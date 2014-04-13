//****************************************** Include *****************************************
#include "zigbee.h"

//************************************* Variables Globales ***********************************

/**
	@var static char file 
	Variable contenant le chemin de l'interface USB avec laquelle on communique.
*/
static char file[16];

int init_ttyUSB(int USB){
	int fid;
	struct termios tty;

	sprintf(file, "/dev/ttyUSB%d", USB);

	if((fid = open(file, O_RDWR | O_NONBLOCK |O_NDELAY)) < 0){
		perror("open failed");
	}

	cfsetospeed(&tty, B9600); //set Baud rate for output
    cfsetispeed(&tty, B9600); //set Baud rate for input

    tty.c_lflag &= ~ICANON; // desable erase, kill, werase and rprnt special character
    tty.c_cc[VMIN] = 1;

    tcflush(fid, TCIFLUSH); // flush device buffer
    
    return tcsetattr(fid, TCSANOW, &tty);
}

int zigbee_w(string message){
	int fid, sizeMessage;

	// Ouverture du fichier
	if((fid = open(file, O_RDWR | O_NONBLOCK | O_NDELAY)) < 0){
		perror("open failed");
		return -1;
	}

	// On fait +1 pour récupérer le nombre de caractère que l'on veut envoyer plus le '\0'
	sizeMessage = message.size() + 1;

	// On envoie le message
	if(write(fid, message.c_str(), sizeMessage) != sizeMessage){
		perror("write failed");
		return -1;
	}

	// On ferme le fichier
	close(fid);
	puts("write success !");
	return 0;
}

int zigbee_r(string &buffer){
	int fid, i = 0;
	char c[2] = "0";

	// Ouverture du fichier
	if ((fid = open(file, O_RDWR | O_NONBLOCK |O_NDELAY)) < 0){
		perror("open failed");
	}

	// On attend de recevoir un '\0' signifiant que la fin du message
	while(c[0] != '\0'){
		// On attend de recevoir un caractère
		while(read(fid, c, 1) != 1);
		buffer[i] = c[0];
		i++;
	}

	printf("%s\n", buffer.c_str());
	// On ferme le fichier
	close(fid);
	puts("Read success !");
	
	return 0;
}