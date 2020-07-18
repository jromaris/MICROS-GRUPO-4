#ifndef DEMODULADOR_FSK_H_
#define DEMODULADOR_FSK_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*demFSKfun_t)(void);

void init_dem_FSK(char* charPtr, demFSKfun_t callback);
//double calc_outSig(void);
bool isDemDataReady(void);
void getDataPackage(bool* data);
void processDemData(void);




#endif /* DEMODULADOR_FSK_H_ */
