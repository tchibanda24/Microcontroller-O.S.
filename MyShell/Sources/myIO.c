/*
 * myIO.c
 *
 * Routines for input and ouput to UART and devices
 *
 *  Created on: Oct 20, 2015
 *      Author: Thabani Chibanda
 */

//determines whether my allocation functions or system functions are used
#define myMem_DISABLE 0

#include "myIO.h"
#include "myMem.h"
#include "mySVC.h"
#include "derivative.h"
#include "pushbutton.h"
#include "led.h"
#include "delay.h"
#include "mcg.h"
#include "sdram.h"
#include "uart.h"
#include "lcdc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pushbutton.h>
#include <led.h>
#include <delay.h>
#include <mcg.h>
#include <sdram.h>
#include <uart.h>

//LED: /user/led/"orange, yellow, blue, green"
//PB: /user//push/"sw1, sw2"
//file: /user/"path"

/*  ********************************            MACROS CONSTANTS           *************************************
 ***************************************************************************************************************  */

#define STRM_META sizeof(stream)
#define FILE_META sizeof(mfile)
#define DATA_META sizeof(fdata)

//Read, Write, Read/Write Flags
#define NUM_MODES 3
#define R_FLAG 0
#define W_FLAG 1
#define RW_FLAG 2

//Device Identifier Flags
#define FILE_DEV 0
#define PUSHB_DEV 1
#define LED_DEV 2
#define ANALOG_DEV 3
#define THERM_DEV 4
#define TOUCH_DEV 5

//PushButtons
#define SW1 0
#define SW2 1

//LED Colors
#define ORANGE 0
#define YELLOW 1
#define GREEN 2
#define BLUE 3

//Touch Sensors
#define TS1 0
#define TS2 1
#define TS3 2
#define TS4 3


/*  ********************************         FUNCTION DECLARATIONS         *************************************
 ***************************************************************************************************************  */

//device functions
static int init_d (stream *strm, int mode, int dn);
static int init_f (stream *strm, int mode, int dn);
static void close_d(stream *strm);
static void close_f(stream *strm);
static char getC(stream *strm);
static char getLED(stream *strm);
static char getPushB(stream *strm);
static char getAnalog(stream *strm);
static char getTherm(stream *strm);
static char getTouch(stream *strm);
static int putC(stream *strm, char c);
static int putLED(stream *strm, char c);
static int putNothing(stream *strm, char c);

//helpers
unsigned int adc_read(uint8_t channel);
int electrode_in(int electrodeNumber);
static int check_name(char *p);
static void addLL(stream *strm);
static void removeLL(stream *strm);
static int find_file(char* name);
static int get_mode(char* mode_char);
int compare_string( char *a, char *b);
static void copy_string(char d[], char s[]);
static int filesz(mfile *f);


/*  ***************************************       TYPE DEFINITIONS         **************************************
 *************************************************************************************************************  */

//struct for stream
typedef struct stream{
	device *dev;
	void *data;
	void *next;
} stream;

//struct for device functions
typedef struct device{
	int (*init)(stream*, int, int);
	char (*read)(stream*);
	int (*write)(stream*, char);
	void (*close)(stream*);
} device;

//file data structure
typedef struct fdata{
	uint8_t mode;
	uint8_t fd;
	uint16_t fn;
	uint16_t oset;
	uint16_t s_size;
} fdata;

//file structure
typedef struct mfile{
	char *name;
	unsigned int type:4;
	unsigned int lock:4;
	uint8_t strm_count;
	uint16_t f_size;
} mfile;

//electrode structure
struct electrodeHW {
	int channel;
	uint32_t mask;
	uint16_t threshold;
	uint16_t *counterRegister;
} electrodeHW[ELECTRODE_COUNT] =
{{5, TSI_PEN_PEN5_MASK, 0, (uint16_t *)&TSI0_CNTR5+1},	/* E1 */
		{8, TSI_PEN_PEN8_MASK, 0, (uint16_t *)&TSI0_CNTR9},	/* E2 */
		{7, TSI_PEN_PEN7_MASK, 0, (uint16_t *)&TSI0_CNTR7+1},	/* E3 */
		{9, TSI_PEN_PEN9_MASK, 0, (uint16_t *)&TSI0_CNTR9+1}};	/* E4 */



/*  ***************************************       Global Variables         ****************************************
 ***************************************************************************************************************  */

//initilize device function pointers
device d_file = {&(init_f), &(getC), &(putC), &(close_f)};
device d_LED = {&(init_d), &(getLED), &(putLED), &(close_d)};
device d_pushB = {&(init_d), &(getPushB), &(putNothing), &(close_d)};
device d_analog = {&(init_d), &(getAnalog), &(putNothing), &(close_d)};
device d_therm = {&(init_d), &(getTherm), &(putNothing), &(close_d)};
device d_touch = {&(init_d), &(getTouch), &(putNothing), &(close_d)};

//create devices ahead of time, mode = 3 for not open
//ddata sw1 = {SW1, 3, 0};
//ddata sw2 = {SW2, 3, 0};
//ddata orange = {ORANGE, 3, 0};
//ddata yellow = {YELLOW, 3, 0};
//ddata green = {GREEN, 3, 0};
//ddata blue = {BLUE, 3, 0};

//devices array
//ddata *devices[] = {&(sw1), &(sw2), &(orange), &(yellow), &(green), &(blue)};

stream *slist = NULL, *slist_b = NULL;      //stream LL pointers
int numstrms[MAX_STREAMS];                  //array used to distribute file descriptors
mfile *files[MAX_FILES];                    //array for holding created files
int numfiles = 0;                           //counts # of total created files
int openfiles = 0;                          //counts # of open files, used as file descriptors

//strings used for checking against input
const int acc_sz = 65;
const char *modes[NUM_MODES] = {"r", "w", "rw"};
const char *devices_str[NUM_DEVS] = {"sw1", "sw2", "orange",  "yellow", "green", "blue", "analog", "thermistor",
    "ts1", "ts2", "ts3", "ts4"}; //+32 for lower->upper case
const char *led_str[NUM_DEVS] = {"orange", "yellow", "green", "blue"};
const char *touch_str[NUM_DEVS] = { "ts1", "ts2", "ts3", "ts4"};
const char* accept = {"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_."};

/*  ************************************              Init Functions             *****************************************
 ***************************************************************************************************************  */

// Initalize the ADC Hardware
void adc_init(void) {
	SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;
	ADC1_CFG1 = ADC_CFG1_MODE(ADC_CFG1_MODE_12_13_BIT);
	ADC1_SC3 = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(ADC_SC3_AVGS_32_SAMPLES);
}

// Initialize the capacitive touch sensors
void TSI_Init(void) {
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_TSI_MASK;
	// Turn on the clock to ports A & B and
	//		to the TSI module
	PORTA_PCR4 = PORT_PCR_MUX(0);		// Enable port A, bit 4 as TSI0_CH5
	PORTB_PCR3 = PORT_PCR_MUX(0);		// Enable port B, bit 3 as TSI0_CH8
	PORTB_PCR2 = PORT_PCR_MUX(0);		// Enable port B, bit 2 as TSI0_CH7
	PORTB_PCR16 = PORT_PCR_MUX(0);		// Enable port B, bit 16 as TSI0_CH9

	TSI0_GENCS |= TSI_GENCS_EOSF_MASK;	// Clear the EOSF (End of Scan) flag

	TSI0_GENCS |= TSI_GENCS_NSCN(10) |	// Set number of consecutive scans per electrode to 11
			TSI_GENCS_PS(4) |			// Set electrode oscillator prescaler to divide by 16
			TSI_GENCS_STPE_MASK |		// Keep TSI running when MCU goes into low power mode
			TSI_GENCS_LPSCNITV(7);		// Low power mode scan interval is set to 50 msec
	TSI0_SCANC |= (TSI_SCANC_EXTCHRG(8) |	// Set ext oscillator charge current to 18 uA
			TSI_SCANC_REFCHRG(15) |		// Set reference osc charge current to 32 uA
			TSI_SCANC_SMOD(10) |		// Set scan period modulus to 10
			TSI_SCANC_AMPSC(1) |		// Divide input clock by 2
			TSI_SCANC_AMCLKS(0));		// Set active mode clock source to LPOSCCLK

	//TSI0_GENCS |= TSI_GENCS_LPCLKS_MASK; // Set low power clock source to VLPOSCCLK

	/* Electrode E1 is aligned with the orange LED */
    #define Electrode_E1_EN_MASK TSI_PEN_PEN5_MASK
	/* Electrode E2 is aligned with the yellow LED */
    #define Electrode_E2_EN_MASK TSI_PEN_PEN8_MASK
	/* Electrode E3 is aligned with the green LED */
    #define Electrode_E3_EN_MASK TSI_PEN_PEN7_MASK
	/* Electrode E4 is aligned with the blue LED */
    #define Electrode_E4_EN_MASK TSI_PEN_PEN9_MASK

	TSI0_PEN = Electrode_E1_EN_MASK | Electrode_E2_EN_MASK |
			Electrode_E3_EN_MASK | Electrode_E4_EN_MASK;

	/* In low power mode only one pin may be active; this selects electrode E4 */
	TSI0_PEN |= TSI_PEN_LPSP(9);
	TSI0_GENCS |= TSI_GENCS_TSIEN_MASK;  // Enables TSI
}

/* Calibrate the capacitive touch sensors */
void TSI_Calibrate(void) {
	int i;
	uint16_t baseline;

	TSI0_GENCS |= TSI_GENCS_SWTS_MASK;	/* Software Trigger Start */
	while(!(TSI0_GENCS & TSI_GENCS_EOSF_MASK)) {
	}
	TSI0_GENCS |= TSI_GENCS_EOSF_MASK;	// Clear the EOSF (End of Scan) flag

	for(i = 0; i < ELECTRODE_COUNT; i++) {
		baseline = *(electrodeHW[i].counterRegister);
		electrodeHW[i].threshold = baseline + THRESHOLD_OFFSET;
	}
}

/*  ************************************          myIO Functions          *****************************************
 ***************************************************************************************************************  */

/*create: takes in a string for @name. This will be the name used to open the file. Allocates memory for the mfile structure, and then another 16 bytes after it to hold the data. The file is created unlocked and with no readers, and a file size of 16. We then enter the address of the file into our @files[] array at the first @NULL position we find.
 */
int mcreate(char *name) {

	mfile *f = malloc(FILE_META + 1);
	int nameCheck = check_name(name);                 //checks the name for valid characters

	if((numfiles == MAX_FILES) || (nameCheck < 0) || !f || (find_file(name) > -1)) return -1;

	//initalize file struct
	f->name = malloc((0xFFFF & nameCheck) + 1);
	copy_string(f->name, name);
	f->type = (nameCheck >> 16);
	f->lock = 0;
	f->strm_count = 0;
	f->f_size = 0;

	//loop to find position for the file in the array
	for(int i = 0; i < MAX_FILES; i++) {
		if(files[i] == NULL) {
			files[i] = f;
			break;
		}
	}

	//we went to set the first position to in the data to end of string
	char *content = (char*)(f + 1);
	content[0] = '\0';

	numfiles++;
	return 1;
}

/*delete: accepts the name of a file to delete, and frees that corresponding file from memory and removes it
        from our file array.

    Returns -1 if file is not found, and -2 if the file is still in use.
 */
int mdelete(char *name) {

	int fn = find_file(name);
	mfile *f = files[fn];

	if(!f) return -1;
	if(f->strm_count > 0) return -2;

	numfiles--;
	files[fn] = NULL;
	free(f->name);
	free(f);
	return 1;
}

/*mopen: accepts the name of a file(preceded by f/ or d/ to denote device) and a mode character.
        Opens a file/device according the parameters. Returns a pointer to the stream for the new file/device

    Returns NULL if file can't be opened/found.
 */
stream* mopen(char* name, char *mode_char) {

	int init_ret = -1;
	int mode = get_mode(mode_char);             //converts the mode from char to integer
	int fn = find_file(name);

	char *out = malloc(30);

	if((mode == -1) || (fn == -1)) return NULL;

	stream *strm = malloc(STRM_META);
	if(strm == NULL) return NULL;

    //switch statement used to determine the appropriate device
	switch((files[fn])->type) {
	case FILE_DEV :
		strm->dev = &(d_file);
		init_ret = (strm->dev)->init(strm, mode, fn);
		if(init_ret != -1) {
			addLL(strm);
			return strm;
		}
		break;
	case PUSHB_DEV :
		strm->dev = &(d_pushB);
		init_ret = (strm->dev)->init(strm, mode, fn);
		if(init_ret != -1) {
			addLL(strm);
			return strm;
		}
		break;
	case  LED_DEV:
		strm->dev = &(d_LED);
		init_ret = (strm->dev)->init(strm, mode, fn);
		if(init_ret != -1) {
			addLL(strm);
			return strm;
		}
	case ANALOG_DEV :
		strm->dev = &(d_analog);
		init_ret = (strm->dev)->init(strm, mode, fn);
		if(init_ret != -1) {
			addLL(strm);
			return strm;
		}
		break;
	case THERM_DEV :
		strm->dev = &(d_therm);
		init_ret = (strm->dev)->init(strm, mode, fn);
		if(init_ret != -1) {
			addLL(strm);
			return strm;
		}
		break;
	default :
		strm->dev = &(d_touch);
		init_ret = (strm->dev)->init(strm, mode, fn);
		if(init_ret != -1) {
			addLL(strm);
			return strm;
		}
		break;
	}

	free(strm);
	return NULL;
}

//mclose: calls the function for closing a stream
void mclose(stream *strm) {

	(strm->dev)->close(strm);
}

//mget: calls the functions for reading from a file/device
char mget(stream *strm) {

	char ch = (strm->dev)->read(strm);
	return ch;
}

//mput: calls the functions for writing @c to a file/device
int mput(stream *strm, char c) {

	int i =  (strm->dev)->write(strm, c);
	return i;
}

//mseek: set a file to the offset specified by the user, -1 sets to the end of the file.
//    Returns position or -1 on failure
int mseek(stream *strm, int offset) {
	if(strm->dev != &d_file) return -1; //dont accept non-file streams

	fdata *dat = strm->data;

	//check for valid offset input, set @dat->oset of the stream if it is
	if((offset != -1) && (offset > dat->s_size)) return -1;

	if(offset == -1)
		dat->oset = dat->s_size - 1;
	else
		dat->oset = offset;

	return dat->oset;
}

//function for the shell to retrieve the file/device descriptor
void mget_desc(stream *strm, int *fd, int *dd) {

	if(strm->dev == &d_file) {
		fdata *dat = strm->data;
		*fd = dat->fd;
	}
	else {
		fdata *dat = strm->data;
		*dd = dat->fd;
	}
}

//lists the files currently in the directory
void mlist() {
	if(numfiles == 0) return;

	mfile *curr_f;

	//print out all file names
	for(int i = 0; i < MAX_FILES; i++){
		if(files[i] != NULL) {
			curr_f = files[i];
			uartPuts(UART2_BASE_PTR, curr_f->name);
			uartPuts(UART2_BASE_PTR, "\t");
		}
	}

	uartPuts(UART2_BASE_PTR, "\n");
}

/*  ********************************         Device Specific Functions        *************************************
 ***************************************************************************************************************  */

/*init: when a device is requested to be opened, make sure it has no other readers or writers and then
        initialze the members of it's struct.

    Returns -1 if device can't be opened
 */
static int init_d (stream *strm, int mode, int fn) {

	mfile *dev = files[fn];
	if(dev->lock == 1) return -1;

	fdata *dat = malloc(sizeof(fdata));
	strm->data = dat;

	dev->lock = 1;
	dev->strm_count = 1;
	dat->mode = mode;
	dat->fn = fn;

	for(int i = 0; i < MAX_STREAMS; i++) {
		if(numstrms[i] == 0) {
			numstrms[i] = 1;
			dat->fd = i;
			break;
		}
	}

	return 1;
}

/*init: Initializes a file requested for opening. Multiple readers will each have their own stream for reading so
        that they can change file position without worrying about it being changed by another reader.
        Returns a file descriptor to the opened file.

    Also increments the amount of readers in the mfile struct
    Returns -1 if device can't be opened, due to file lock or write request to a file being read
 */
static int init_f (stream *strm, int mode, int fn) {

	mfile *f = files[fn];
	if(f->lock == 1) return -1;

	//if: file has no readers/writers, we can go ahead and create the struct
	//else if: file is opened, we ony want to initialize another stream for reading
	if(f->strm_count == 0) {
		fdata *newfdata = malloc(sizeof(fdata));
		strm->data = newfdata;

		//find this stream the lowest file descriptor not already in use
		for(int i = 0; i < MAX_STREAMS; i++) {
			if(numstrms[i] == 0) {
				numstrms[i] = 1;
				newfdata->fd = i;
				break;
			}
		}

		//initalize stream data, update counters, return file descriptor
		newfdata->fn = fn;
		newfdata->mode = mode;
		newfdata->s_size = filesz(f);
		newfdata->oset = 0;
		if(mode != R_FLAG) f->lock = 1;

		(f->strm_count)++;
		openfiles++;
		return newfdata->fd;
	}
	else if (mode == R_FLAG) {
		fdata *newfdata = malloc(sizeof(fdata));
		strm->data = newfdata;

		//find this stream the lowest file descriptor not already in use
		for(int i = 0; i < MAX_STREAMS; i++) {
			if(numstrms[i] == 0) {
				numstrms[i] = 1;
				newfdata->fd = i;
				break;
			}
		}

		//initalize stream data, update counters, return file descriptor
		newfdata->fn = fn;
		newfdata->mode = mode;
		newfdata->s_size = filesz(f);
		newfdata->oset = 0;

		(f->strm_count)++;
		openfiles++;
		return newfdata->fd;
	}

	return -1;
}

/*close_(f/d): For files and devices: remove the stream from our linked list and free the data and stream
              structures that were allocated.  Nothing is returned, as we do nothing without a stream.

    For devices: we set everything back to it's original value. Devices are NOT turned off if left on.

    For files: unlock the file(no need to check mode, always setting to zero covers all cases), decremnt open, free up file descriptor.

 */
static void close_f(stream *strm) {
	if(!strm) return;

	fdata *dat = strm->data;
	mfile *f = files[(dat->fn)];

	//updata structs, free memory
	f->lock = 0;
	(f->strm_count)--;
	numstrms[dat->fd] = 0;
	removeLL(strm);

	strm->dev = NULL;
	free(dat);
	free(strm);
}

//device closer
static void close_d(stream *strm) {
	if(!strm) return;

	fdata *dat = strm->data;
	mfile *dev = files[(dat->fn)];

	//updata structs, free memory
	dev->lock = 0;
	dev->strm_count = 0;
	dat->mode = 3;
	numstrms[dat->fd] = 0;
	removeLL(strm);

	strm->dev = NULL;
	strm->data = NULL;
	free(strm->data);
	free(strm);
}

//getChar: gets the next character in the file and returns it to the user. Increments the file pointer to the
//        next character. Returns '\0' for EOF and '\b' denotes a file being written to
static char getC(stream *strm) {
	if(!strm) return -1;

	char ch;
	fdata *dat = strm->data;
	if(dat->mode == W_FLAG) return '\b';

	mfile *f = files[dat->fn];
	char *content = (char*)(f + 1);

	ch = content[(dat->oset)];
	//if(ch == '\0') return -1;

	(dat->oset)++;
	return ch;
}

//get'DEV': each of these return the state of the device specified by the user
static char getLED(stream *strm) {
	if(!strm) return -1;

	fdata *dat = strm->data;
	if(dat->mode == W_FLAG) return -1;

	return dat->oset;
}

static char getPushB(stream *strm) {
	if(!strm) return -1;

	int ret = -1;
	fdata *dat = strm->data;
	if(dat->mode == W_FLAG) return -1;

	mfile *pb = files[dat->fn];

	if(compare_string(pb->name, "sw1") == 1)
		ret = sw1In();
	else if(compare_string(pb->name, "sw2") == 1)
		ret = sw2In();

	return ret;
}

static char getAnalog(stream *strm) {
	if(!strm) return -1;

	fdata *dat = strm->data;
	if(dat->mode == W_FLAG) return -1;

	int ret;
	ret = (int)adc_read(ADC_CHANNEL_POTENTIOMETER);

	return ret;
}

static char getTherm(stream *strm) {
	if(!strm) return -1;

	fdata *dat = strm->data;
	if(dat->mode == W_FLAG) return -1;

	int ret;
	ret = (int)adc_read(ADC_CHANNEL_TEMPERATURE_SENSOR);

	return ret;
}

static char getTouch(stream *strm) {
	if(!strm) return -1;

	int touchi = 0;
	int ret;
	fdata *dat = strm->data;
	mfile *dev = files[dat->fn];
	if(dat->mode == W_FLAG) return -1;

	for(int i = 0; i < 4; i++) {
		if(compare_string(dev->name, touch_str[i]) == 1) break;
		touchi++;
	}

	switch(touchi) {
	case TS1 :
		ret = electrode_in(TS1);
		break;
	case  TS2:
		ret = electrode_in(TS2);
		break;
	case TS3:
		ret = electrode_in(TS3);
		break;
	default :
		ret = electrode_in(TS4);
		break;
	}

	return ret;
}

/*putChar: Writes a character, @c, from the user into the specified file, at the current position.
          Files have a set start size, so if that size is met, the file is extended by one double word
          up until the maximum size, which is 65,000 bytes

    return -1 if it fails, -2 if the file stream is open for reading
 */
static int putC(stream *strm, char c) {
	if(!strm) return -1;

	fdata *dat = strm->data;
	if(dat->mode == R_FLAG) return -2;

	mfile *f = files[dat->fn];
	char *content = (char*)(f + 1);

	//if: our file has run out of pre allocated space, extend it if we can
	//else: write to the file at positon @dat->oset
	if((dat->oset + 1) == f->f_size){

		if((dat->s_size + 16) > MAX_FILE_SIZE) return -1;

		//allocate a replacement fail 16 bytes longer
		mfile *replace = malloc(FILE_META + dat->s_size + 16);
		char* new_content = (char*)(replace + 1);

		if(!replace) return -1;

		//copy over old file data into the replacement
		*replace = *f;
		replace->f_size = replace->f_size + 16;
		copy_string(new_content, content);

		//write to the new file
		content[dat->oset] = c;
		if((dat->s_size) == (dat->oset)) {
			content[dat->oset + 1] = '\0';
			(dat->s_size)++;
		}
		(dat->oset)++;

		//replace old file in the file array, and free it
		files[dat->fn] = replace;
		free(f);
	}
	else {
		content[dat->oset] = c;

		//only increase the stream file size tracker when we write to the end
		if((dat->s_size) == (dat->oset)) {
			content[dat->oset + 1] = '\0';
			(dat->s_size)++;
		}

		(dat->oset)++;
	}

	return 1;
}

//putDev: either turn a device on or of. 0 = off, 1 = on. Returns 1
static int putLED(stream *strm, char c) {
	if(!strm) return -1;

	int ledi = 0;
	fdata *dat = strm->data;
	mfile *dev = files[dat->fn];
	if(dat->mode == R_FLAG) return -1;

    //determine the particular LED
	for(int i = 0; i < 4; i++) {
		if(compare_string(dev->name, led_str[i]) == 1) break;
		ledi++;
	}

    //if 0: switch statement determines the LED and turns it off
	if(c == '0') {
		switch(ledi) {
		case ORANGE :
			ledOrangeOff();
			break;
		case  YELLOW:
			ledYellowOff();
			break;
		case GREEN:
			ledGreenOff();
			break;
		default :
			ledBlueOff();
			break;
		}

		dat->oset = 0;
	}
    //if 1: switch statement determines the LED and turns it on
	else if(c == '1') {
		switch(ledi) {
		case ORANGE :
			ledOrangeOn();
			break;
		case  YELLOW:
			ledYellowOn();
			break;
		case GREEN:
			ledGreenOn();
			break;
		default :
			ledBlueOn();
			break;
		}

		dat->oset = 1;
	}
	else
		return -1;

	return 1;
}

static int putNothing(stream *strm, char c) {
	if(!strm) return -1;
	return 1;
}

/*  ************************************            Helper Functions          *************************************
 ***************************************************************************************************************  */


//function for reading from ADC Channel
unsigned int adc_read(uint8_t channel) {
	ADC1_SC1A = channel;
	while(!(ADC1_SC1A & ADC_SC1_COCO_MASK)) {
	}
	return ADC1_RA;
}

//function for reading from touch sensor
int electrode_in(int electrodeNumber) {
	uint16_t oscCount;

	TSI0_GENCS |= TSI_GENCS_SWTS_MASK;	        /* Software Trigger Start */
	while(!(TSI0_GENCS & TSI_GENCS_EOSF_MASK)) {
	}
	TSI0_GENCS |= TSI_GENCS_EOSF_MASK;	        // Clear the EOSF (End of Scan) flag

	oscCount = *(electrodeHW[electrodeNumber].counterRegister);

	/* Returns 1 when pushbutton is depressed and 0 otherwise */
	return oscCount > electrodeHW[electrodeNumber].threshold;
}

/*check_name: Checks that a user-inputted filename is valid according to the documentation.
              Returns the length of the name entered.

    Returns -1 if the filename is longer than 32 characters, or has letter not in the @accept string
 */
static int check_name(char *p) {

	int ret = 0, dot = 0, curr = 0;

	//while our name is not at the end, check each char againt our @accept string chars
	while((p[ret] != '\0')) {
		curr = 0;

		//loop through all @accept string chars
		for(int j = 0; j < acc_sz; j++) {
			//if we have a valid char, set curr = 1(character accepted) and break
			if(p[ret] == accept[j]) {
				curr = 1;

				//upon reading a '.', check to make sure not others were read
				if(p[ret] == '.') {
					dot++;
					if(dot > 1) return -1;
				}
				break;
			}
		}
		if(curr == 0) return -1;

		ret++;
		if((ret == 32) && (p[ret] != '\0')) return -1;  //limits the length of a name to 32 char
	}

	if(curr < 1) return -1;        //return failure

	for(int i = 0; i < NUM_DEVS; i++){
		curr = compare_string(devices_str[i], p);

		if(curr == 1) {
			if(i < 2)
				curr = PUSHB_DEV;
			else if(i < 6)
				curr = LED_DEV;
			else if(i < 7)
				curr = ANALOG_DEV;
			else if(i < 8)
				curr = THERM_DEV;
			else
				curr = TOUCH_DEV;

			ret = (ret & 0x0000FFFF) | ((curr & 0xFF) << 16);
			break;
		}
		else{
			curr = 0;
		}
	}

	return ret;
}

//addLL: adds the stream passed to it to the LL
static void addLL(stream *strm) {

	if(slist == NULL) {
		slist = strm;
		slist_b = strm;
		strm->next = NULL;
		return;
	}
	else{
		slist_b->next = strm;
		slist_b = strm;
		strm->next = NULL;
	}

	return;
}

//removeLL: removes the stream passed to it to the LL
static void removeLL(stream *strm) {

	stream *prev = NULL;
	stream *curr = slist;

	while(curr) {
		if(curr == strm){

			//if removed node is top/bottom then update @slist/slist_b
			if(curr == slist) {
				slist = curr->next;
				if(curr == slist_b)
					slist_b = slist;
			}
			else {
				if(curr == slist_b)
					slist_b = prev;

				prev->next = curr->next;
			}
			return;
		}
		else {
			prev = curr;
			curr = curr->next;
		}
	}
}

/*find_file: Iterates through the file array, looking for the file w/ the name passed to it.
                Returns the index of the file in the array, or -1 if not found

 */
static int find_file(char* name){

	int cmp = 0;

	//iterate through files, compare names, return index upon comparison success
	for(int i = 0; i < numfiles; i++){
		char *currfile = (files[i])->name;

		cmp = compare_string(name, currfile);
		if(cmp == 1) return i;
	}

	return -1;
}

/*get mode: Iterates through the mode array, looking for the mode w/ the string passed to it
                Returns the index of the mode for the flag or -1 if not found
 */
static int get_mode(char* mode_char){

	int cs = 0;

	//iterate through mode array, compare strings, return index(mode) upon comparison success
	for(int i = 0; i < NUM_MODES; i++){
		char *currmode = (char*)(modes[i]);

		cs = compare_string(mode_char, currmode);
		if(cs == 1) return i;
	}

	return -1;
}

//functions for comparing two strings, return 1 on success and -1 on failure
int compare_string(char *a, char *b) {

	int i = 0;

	while((a[i] != '\0') && (b[i] != '\0')) {
		if(a[i] == b[i]){
			i++;

			if((a[i] == '\0') && (b[i] == '\0')) return 1;
		}
		else{
			break;
		}
	}

	return 0;
}

//implentation of a string copy function to replace 'strcpy'
void copy_string(char *d, char *s) {
	int c = 0;

	while (s[c] != '\0') {
		d[c] = s[c];
		c++;
	}

	d[c] = '\0';
}

//finds the size of a file
static int filesz(mfile *f) {

	int i = 0;
	char *content = (char*)(f + 1);

	while(content[i] != '\0'){
		i++;
	}

	return i;
}
