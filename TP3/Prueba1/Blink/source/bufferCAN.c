#include <bufferCAN.h>


void init_bufferCAN(circ_buff_t * buff,int len)
{
	buff->len=len;
	buff->head=0;
	buff->tail=0;
}

void  push_bufferCAN(circ_buff_t * buff, CANmsg data)
{
	int next;
	next = buff->head + 1;  // next is where head will point to after this write.
	if (next >= buff->len)
	    next = 0;
	buff->buffer[buff->head] = data;  // Load data and then move
	buff->head = next;             // head to next data offset.
}

CANmsg pop_bufferCAN(circ_buff_t * buff)
{
	CANmsg data;
	int next;
	next = buff->tail + 1;  // next is where tail will point to after this read.
	if(next >= buff->len)
		next = 0;
	data=buff->buffer[buff->tail];
	buff->tail = next;              // tail to next offset.
	return data;
}
int buffer_is_fullCAN(circ_buff_t * buff)
{
	int next;
	    next = buff->head + 1;  // next is where head will point to after this write.
	    if (next >= buff->len)
	        next = 0;

	    if (next == buff->tail)  // if the head + 1 == tail, circular buffer is full
	        return 1;
	    else
	    	return 0;
}

int buffer_is_emptyCAN(circ_buff_t * buff)
{
	if (buff->head == buff->tail)  // if the head == tail, we don't have any data
		return 1;
	else
		return 0;

}
