#ifndef SPI_H_
#define SPI_H_

#include<stdint.h>
#include<stdbool.h>

//Función que inicializa el periférico de SPI.
void SPI_init (void);

//Función que manda y recibe la cantidad de bytes deseada por SPI. El puntero data2send debe estar preparado para mandar data_len bytes. Si se desea leer el MISO receivedData tiene que ser un puntero no NULL.
char SPI_sendReceive(char * data2send, char data_len,char * recievedData);

#endif /* SPI_DRIVER_H_ */
