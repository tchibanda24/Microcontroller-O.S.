/*
 * myIO.h
 *
 *  Created on: Oct 20, 2015
 *      Author: Thabani Chibanda
 */

#include <stdlib.h>
#include <stdio.h>

#define MAX_STREAMS 255
#define MAX_FILES 6499
#define MAX_FILE_SIZE 64999
#define NUM_DEVS 13

//ADC Specific
#define ADC_CHANNEL_POTENTIOMETER   	0x14
#define ADC_CHANNEL_TEMPERATURE_SENSOR  0x1A

#define ADC_CFG1_MODE_8_9_BIT       0x0
#define ADC_CFG1_MODE_12_13_BIT     0x1
#define ADC_CFG1_MODE_10_11_BIT     0x2
#define ADC_CFG1_MODE_16_BIT        0x3
#define ADC_SC3_AVGS_32_SAMPLES     0x3

//Touch Sensor Specific
#define PORT_PCR_MUX_ANALOG 0
#define PORT_PCR_MUX_GPIO 1
#define ELECTRODE_COUNT 4
#define THRESHOLD_OFFSET 0x200

//typedefintions
typedef struct stream stream;
typedef struct device device;
typedef struct ddata ddata;
typedef struct fdata fdata;
typedef struct mfile mfile;

//necessary functions
int mcreate(char *name);
int mdelete(char *name);
stream* mopen(char* name, char *mode_char);
void mclose(stream *strm);
char mget(stream *strm);
int mput(stream *strm, char c);
int mseek(stream *strm, int offset);
//int mflush(stream *strm);
void mget_desc(stream *strm, int *fd, int *dd);
int compare_string(char *a, char *b);
void mlist();

//initalizers
void adc_init(void);
void TSI_Init(void);
void TSI_Calibrate(void);
