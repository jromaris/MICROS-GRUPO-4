#ifndef BUFFER_H_
#define BUFFER_H_

#define BUFFER_SIZE 100

typedef struct CANdata {
   int id;
   char size;
   char data[10];
} CANmsg;

typedef struct {
	CANmsg buffer[BUFFER_SIZE];
    int len;
    int head;
    int tail;
} circ_buff_t;
/*
 INICIALIZO EL BUFFER
*/
void init_bufferCAN(circ_buff_t * buff,int len);

/*
PUSHEO ELEMENTO AL BUFFER, HAY QUE CHEQUEAR DE ANTEMANO QUE EL BUFFER NO ESTÉ LLENO
*/
void push_bufferCAN(circ_buff_t * buff, CANmsg data);

/*
POPPEO ELEMENTO DEL BUFFER, HAY QUE CHEQUEAR DE ANTEMANO QUE EL BUFFER NO ESTÉ VACÍO
*/
CANmsg pop_bufferCAN(circ_buff_t * buff);

/*
DEVUELVE 1 SI EL BUFFER ESTÁ LLENO, 0 DE LO CONTRARIO
*/
int buffer_is_fullCAN(circ_buff_t * buff);

/*
DEVUELVE 1 SI EL BUFFER ESTÁ VACÍO, 0 DE LO CONTRARIO
*/
int buffer_is_emptyCAN(circ_buff_t * buff);

#endif /* FSMTABLE_H_ */
#pragma once
