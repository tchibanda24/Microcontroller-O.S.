/*
 * myShell.c
 *
 * Shell commands and test functions
 *
 *  Created on: Sept 9, 2015
 *      Author: Thabani Chibanda
 */

#include "myShell.h"
#include "derivative.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

//helper function declaration
stream *get_strm(char *argv[], char *type);
int string_to_num(char *p);
int power(int base, unsigned int exp);
void copy_string(char d[], char s[]);

/* ****************************                GLOBAL VARIABLES            ************************************
   ************************************************************************************************************  */

//Error enumerator and corresponding strings
enum {
	ERR_INVALID_CMD,
	ERR_ARGS,
	ERR_CMD_FAIL,
	ERR_HELP,
	ERR_NO_FILE,
	ERR_FILE_IN_USE,
	ERR_CREATE,
	ERR_OPEN,
	ERR_EOF
} errors;

//command structure: keeps the name of the command, it's calling function, and it's description for the user
static struct commandEntry {
	char *name;
	int (*functionp)(int argc, char *argv[]);
	char *desc;
}  commands[] = {{"create", cmd_create, "help: Accepts one argument, a file name(can only contain numbers, letters, underscore, dash, and accepts any extension). A file in memory will be created with that name.\n"},
		{"delete", cmd_delete, "help: Accepts one argument, the name of an existing file in memory. Will delete the file only if it is not currently opened. File cannot be accesed after being deleted.\n"},
		{"open", cmd_open, "help: Accepts two arguments, the name of an existing file/device('f/filename.txt' - 'd/devicename') in memory and how you would like that file/device to be opened(r = read, w = write, rw = read/write) : devicenames: 'sw1'-'sw2'-'orange'-'yellow'-'green'-'blue'.\n-Files will be accessed from the beginning, so any data currently in the file will be overwritten if opened for writing.\n-Only one writer allowed at one time, but multiple readers are allowed. A file can not be written to while being read, and vice versa.\n-Will return a file descriptor for handling files.\n"},
		{"close", cmd_close, "help: Accepts one argument, the name of an existing file in memory. Will close the file only for the stream passed to it.\n"},
		{"read", cmd_read, "help: Accepts one argument, a file descripter or device name. Displays either the current character pointed to in a file(and slides that pointer down) or the current state of a device.\n"},
		{"write", cmd_write, "help: Accepts two argumentsa, file/device  descripter along with a single character/integer for writing.\n-For devices: '0' = off, '1' = on\n"},
		{"seek", cmd_seek, "help: Accepts two arguments, the name of an existing file in memory and a position in that file to be read or written to. '-1' can be entered to be put at the end of the file.\n"},
		{"ls", cmd_list, "help: Accepts no arguments. Returns the name of files in directory.\n"},
		{"malloc", cmd_malloc, "help: Accepts one integer argument. Allocates memory for amount specified, and returns address of the memory.\n"},
		{"free", cmd_free, "help: Accepts one argument. Frees a previously allocated block of memory at the address given.\n"},
		{"mmap", cmd_mmap, "date: Accepts no arguments. Outputs the current addresses allocated by the shell.\n"},
		{"echo", SVCEcho, "echo: Accepts any amount of arguments. Outputs, or \"echoes\" the arguments you enter along with it.\n"},
		{"date", cmd_date, "date: Accepts no arguments. Outputs the current date and time.\n"},
		{"exit", cmd_exit, "exit: Accepts no arguments. Quits the shell program.\n"},
		{"help", cmd_help, "help: Accepts one or no arguments. Gives descriptions of the available commands.\n"},
		{"ser2lcd", cmd_ser2lcd, "ser2lcd: Accepts no arguments. Continuously copies characters from input to LCD.  Ends on a ^D (control-D) input character.\n"},
		{"touch2led", cmd_touch2led, "touch2led: Accepts no arguments. Continuously copies from each touch sensor to the corresponding LED.  Stops when all four touch sensors are  depressed.\n"},
		{"pot2ser", cmd_pot2ser, "pot2ser: Accepts one or no arguments. Continuously outputs the value of the analog potentiomemter to the serial device as a decimal(with no arguments or 'dec' as an argument) or hexadecimal(with 'hex' as an argument) number followed by a newline.\n"},
		{"therm2ser", cmd_therm2ser, "therm2ser: Accepts one or no arguments. Continuously outputs the value of the analog thermistor device to the serial device as a decimal(with no arguments or 'dec' as an argument) or hexadecimal(with 'hex' as an argument) number followed by a newline..\n"},
		{"pb2led", cmd_pb2led, "pb2led: Accepts no arguments. Continuously copies from SW1 to orange LED and SW2 to  yellow LED.  Stops when both SW1 and SW2 are depressed..\n"},
		{"intTest1", cmd_intTest1, "intTest1: copies from UART2 input to the LCD display using device independent supervisor calls.\n"},
		{"intTest2", cmd_intTest2, "intTest2: sends a message over UART2 output whenever pushbutton S2 is depressed.\n"},
		{"TestMLFQ", cmd_intTest3, "intTest3: uses the supervisor call for user timer events, flash the orange LED on and off every half a second (the LED will light once a second).\n"},
};

stream *open_f[MAX_STREAMS];
stream *open_d[NUM_DEVS];


//serial port 2 input character buffer
extern char SerPort2InBuffer[IO_BUFFER_SIZE];
extern int s2InEnq;
extern int s2InDeq;
extern int s2InCount;

//serial port 2 output character buffer
extern char SerPort2OutBuffer[IO_BUFFER_SIZE];
extern int s2OutEnq;
extern int s2OutDeq;
extern int s2OutCount;

//interrupt counters
extern int intCount;
extern int intTDRECount;
extern int intRDRFCount;
extern int intNotDRCount;

const int IRC = 32000;					/* Internal Reference Clock */
const int FLL_Factor = 640;
const int KHzInHz = 1000;
const int peripheralClock = 60000000;
const int baud = 115200;

/* ******************************               INIT FUNCTIONS                *********************************
   ************************************************************************************************************  */

// Initialize the resourced used used by the shell
void shell_Init(const int moduleClock) {

	mcgInit();
	sdramInit();
	lcdcInit();
	lcdcConsoleInit(&console);
	ledInitAll();
	switchcmdInit();
	uartInit(UART2_BASE_PTR, peripheralClock/KHzInHz, baud);
	adc_init();
	TSI_Init();
	TSI_Calibrate();
	svcInit_SetSVCPriority(10);
	flexTimer0Init(1875);

	//enable interrupts
	UART2_C2 |= UART_C2_RIE_MASK;
	NVICEnableIRQ(UART2_STATUS_IRQ_NUMBER, UART2_STATUS_INTERRUPT_PRIORITY);
}


/* **********************************          COMMAND FUNCTIONS          ************************************
   ***********************************************************************************************************  */

//cmd_create: creates a file from the second argument, returns error if myIO fails
int cmd_create(int argc, char *argv[]) {
	//Check arguments
	if(argc != 2){
		return ERR_ARGS;
	}

	char * out = malloc(50);

	sprintf(out, "filename: %s\r\n", argv[1]);
	uartPuts(UART2_BASE_PTR, out);
	lcdcConsolePuts(&console, out);

	int check = mcreate(argv[1]);

	free(out);
	if(check == -1) return ERR_CREATE;
	return 0;
}

//cmd_delete: deletes the file from memory, returns erros if it's in use or doesn't exist
int cmd_delete(int argc, char *argv[]){
	//Check arguments
	if(argc != 2){
		return ERR_ARGS;
	}

	char *name = argv[1];
	int check = mdelete(name);

	if(check == -1) return ERR_NO_FILE;
	if(check == -2) return ERR_FILE_IN_USE;
	return 0;
}

//cmd_open: calls mopen, takes the pointer(err if NULL) and enters it into an array where we store all open files.
int cmd_open(int argc, char *argv[]) {
	//Check arguments
	if(argc != 3){
		return ERR_ARGS;
	}

	stream *strm;
	int fd = -1, dd = -1;
	char *name = argv[1];
	char *mode = argv[2];

	strm = mopen(name, mode);
	if(strm == NULL) return ERR_OPEN;

	//retrieve file/device descriptor for entering pointer into array
	mget_desc(strm, &fd, &dd);

	char *out = malloc(30);

	if(fd != -1) {
		open_f[fd] = strm;
		sprintf(out, "File opened. file descriptor: f%d\r\n", fd);
		uartPuts(UART2_BASE_PTR, out);
		lcdcConsolePuts(&console, out);
		return 0;
	}
	else if(dd != -1) {
		open_d[dd] = strm;
		sprintf(out, "Device opened. device descriptor: d%d\r\n", dd);
		uartPuts(UART2_BASE_PTR, out);
		lcdcConsolePuts(&console, out);
		return 0;
	}

	free(out);
	return ERR_CMD_FAIL;
}

//cmd_close: closes a stream opened by the user, returns error if it doesn't exist
int cmd_close(int argc, char *argv[]){
	//Check arguments
	if(argc != 2){
		return ERR_ARGS;
	}

	char type = '\0';
	int fd = -1, dd = -1;
	stream *strm = get_strm(argv, &type);
	if(strm == NULL) return ERR_NO_FILE;

	mget_desc(strm, &fd, &dd);

	//setvbuf(stdout, NULL, _IONBF, 0); // no buffering on stdout - for printf()

	if(fd != -1) {
		uartPuts(UART2_BASE_PTR, "File closed\r\n");
		lcdcConsolePuts(&console, "File closed\r\n");
		open_f[fd] = NULL;
	}
	else if(dd != -1) {
		uartPuts(UART2_BASE_PTR, "Device closed.\r\n");
		lcdcConsolePuts(&console, "Device closed\r\n");
		open_d[dd] = NULL;
	}

	mclose(strm);
	return 0;
}

//cmd_read: calls myIO read function, will display the results of the call. Err for no stream from input
int cmd_read(int argc, char *argv[]) {
	//Check arguments
	if(argc != 2){
		return ERR_ARGS;
	}

	char type = '\0';
	stream *strm = get_strm(argv, &type);
	if(strm == NULL) return ERR_NO_FILE;

	char ret = mget(strm);

	if(type == 'f') {
		if(ret == '\0') return ERR_EOF;
		if(ret == '\b') return ERR_CMD_FAIL;
		char *out = malloc(80);
		sprintf(out, "output: %c", ret);
		uartPuts(UART2_BASE_PTR, out);
		lcdcConsolePuts(&console, out);
		free(out);
	}
	else if(type == 'd') {
		if(ret == 1) {
			uartPuts(UART2_BASE_PTR, "Device is on/active.\r\n");
			lcdcConsolePuts(&console, "Device is on/active.\r\n");
		}
		else {
			uartPuts(UART2_BASE_PTR, "Device is off.\r\n");
			lcdcConsolePuts(&console, "Device is off.\r\n");
		}
	}

	return 0;
}

//cmd_write: writes the 3rd argument(a char) to the get function in myIO, doens't display success but err is reported
int cmd_write(int argc, char *argv[]){
	//Check arguments
	if(argc != 3){
		return ERR_ARGS;
	}

	char *in = argv[2];
	if(in[1] != '\0') return ERR_CMD_FAIL;

	char type = '\0';
	stream *strm = get_strm(argv, &type);
	if(strm == NULL) return ERR_NO_FILE;

	int check = mput(strm, in[0]);
	if(check == -1) return ERR_CMD_FAIL;
	if(check == -2) return ERR_FILE_IN_USE;

	return 0;
}

//cmd_seek: calls the mseek function, displays new position or error on failure
int cmd_seek(int argc, char *argv[]) {
	//Check arguments
	if(argc != 3){
		return ERR_ARGS;
	}

	char type = '\0';
	int offset = 0;

	if(compare_string(argv[2], "-1") == 1) {
		offset = -1;
	}
	else {
		offset = string_to_num(argv[2]);
		if(offset == -1) return ERR_CMD_FAIL;
	}


	stream *strm = get_strm(argv, &type);
	if(strm == NULL || type == 'd') return ERR_NO_FILE;

	int check = mseek(strm, offset);

	if(check == -1)
		return ERR_CMD_FAIL;

	char * out = malloc(30);
	sprintf(out, "New File Position: %d\r\n", check);
	uartPuts(UART2_BASE_PTR, out);
	lcdcConsolePuts(&console, out);

	free(out);
	return 0;
}

//cmd_list: lists files in the directory
int cmd_list(int argc, char *argv[]) {
	//Check arguments
	if(argc != 1){
		return ERR_ARGS;
	}

	mlist();
	return 0;
}

//malloc command that allocates a specified amount of memory and returns it's address
int cmd_malloc(int argc, char *argv[]){
	//Check arguments
	if(argc != 2){
		return ERR_ARGS;
	}

	int space = string_to_num(argv[1]);
	void *addr = NULL;

	addr = malloc(space);

	if(addr) {
		char * out = malloc(30);
		sprintf(out, "Address: %p\r\n", addr);
		uartPuts(UART2_BASE_PTR, out);
		lcdcConsolePuts(&console, out);

		free(out);
		return 0;
	}
	else
		return ERR_CMD_FAIL;
}

//command that frees a specified block of allocated of memory
int cmd_free(int argc, char *argv[]){
	//Check arguments
	if(argc > 2){
		return ERR_ARGS;
	}

	char *input = argv[1];

	unsigned int result = 0;
	int c ;

	//**had a hard time coming up w/ this so I got most of this idea online
	if (('0' == *input) && ('x' == *(input+1))) {
		input+=2;

		while (*input) {

			result = result << 4;

			if (c = (*input-'0' ) , (c>=0 && c <=9))
				result|=c;
			else if (c=(*input-'A'),(c>=0 && c <=5))
				result|=(c+10);
			else if (c=(*input-'a'),(c>=0 && c <=5))
				result|=(c+10);
			else break;
			++input;
		}
	}

	void *ptr = (void*)result;

	free(ptr);
	uartPuts(UART2_BASE_PTR, "Memory freed.\r\n");
	lcdcConsolePuts(&console, "Memory freed.\r\n");

	return 0;
}

//outputs the address of all currently malloc'd addresses
int cmd_mmap(int argc, char *argv[]) {
	//Check arguments
	if(argc > 1) {
		return ERR_ARGS;
	}

	mmap();
	return 0;
}

//Repeats the users programs to them
int cmd_echo(int argc, char *argv[]){

	if(argv) {
		for(int i = 1; i < argc; i++) {
			char * out = malloc(80);
			sprintf(out, " %s ", argv[i]);
			uartPuts(UART2_BASE_PTR, out);
			lcdcConsolePuts(&console, out);
		}
	}

	uartPuts(UART2_BASE_PTR, "\r\n");
	lcdcConsolePuts(&console, "\r\n");
	return 0;
}

//Outputs the time and date, no commands taken
//ran out of time and used some functions that were supposed to be omitted
int cmd_setDate(int argc, char *argv[]) {
	//Check arguments
	if(argc > 3){
		return 1;
	}

	flexTimer0Start();

	char *date = argv[1];
	char *time = argv[2];

	return 0;
}

int cmd_getDate(int argc, char *argv[]) {
	//Check arguments
	if(argc > 1){
		return 1;
	}

	return 0;
}

int cmd_date(int argc, char *argv[]) {
	//Check arguments
	if(argc > 1){
		return 1;
	}

	return 0;
}

//Gives a run down of all commands for the user, can accept only  command as an argument
int cmd_help(int argc, char *argv[]) {
	//Checks the arguments entered. For one, gives a general help statement
	//For two arguments, confirms the second is a command then gives description
	//For more than two returns an error code
	if(argc == 1){
		uartPuts(UART2_BASE_PTR, "This is a shell implementation. Type commands and arguments, and hit enter.\r\n");
		lcdcConsolePuts(&console, "This is a shell implementation. Type commands and arguments, and hit enter.\r\n");
		uartPuts(UART2_BASE_PTR, "The following are built in: 'date' 'echo' 'malloc' 'free' 'mmap' 'exit' 'help'. Enter help (command) for more on each.\r\n");
		lcdcConsolePuts(&console, "The following are built in: 'date' 'echo' 'malloc' 'free' 'mmap' 'exit' 'help'. Enter help (command) for more on each.\r\n");
		return 0;
	}
	else if(argc == 2){

		int cmd_entered = -1;

		for(int i = 0; i < 4; i++){
			if(strcmp(argv[1], commands[i].name) == 0)
				cmd_entered = i;
		}

		if(cmd_entered != -1) {
			char * out = malloc(80);
			sprintf(out, " %s\r\n", (commands[cmd_entered].desc));
			uartPuts(UART2_BASE_PTR, out);
			lcdcConsolePuts(&console, out);
			free(out);
		}
		else {
			return ERR_HELP;
		}
	}
	else if(argc > 2){
		return ERR_ARGS;}

	return 0;
}

//Exits the shell, no commands taken
int cmd_exit(int argc, char *argv[]){
	//Check arguments
	if(argc > 1){
		return 1;
	}

	exit(0);
	return 0;
}


/* **********************************           TEST FUNCTIONS           **************************************
   ************************************************************************************************************  */

//1. ser2lcd: Continuously copy characters from serial input to LCD.  End on a ^D (control-D) input character.
int cmd_ser2lcd(int argc, char *argv[]){
	//Check arguments
	if(argc > 1){
		return 1;
	}

	char ch;

	while(1) {
		while(!uartGetcharPresent(UART2_BASE_PTR))
			delay(1);

		ch = uartGetchar(UART2_BASE_PTR);

		if(ch == CHAR_EOF)
			break;

		uartPutchar(UART2_BASE_PTR, ch);
		lcdcConsolePutc(&console, ch);
	}

	return 0;
}


//2. touch2led: Continuously copy from each touch sensor to the corresponding LED.  End when all four touch sensors are "depressed."
int cmd_touch2led(int argc, char *argv[]){
	//Check arguments
	if(argc > 1){
		return 1;
	}

	mcreate("ts1");
	mcreate("ts2");
	mcreate("ts3");
	mcreate("ts4");
	mcreate("orange");
	mcreate("yellow");
	mcreate("green");
	mcreate("blue");
	stream* s1 = mopen("ts1", "r");
	stream* s2 = mopen("ts2", "r");
	stream* s3 = mopen("ts3", "r");
	stream* s4 = mopen("ts4", "r");
	stream* s5 = mopen("orange", "w");
	stream* s6 = mopen("yellow", "w");
	stream* s7 = mopen("green", "w");
	stream* s8 = mopen("blue", "w");

	while(1) {

		delay(1);

		if(mget(s1))
			mput(s5, '1');
		else
			mput(s5, '0');

		if(mget(s2))
			mput(s6, '1');
		else
			mput(s6, '0');

		if(mget(s3))
			mput(s7, '1');
		else
			mput(s7, '0');

		if(mget(s4))
			mput(s8, '1');
		else
			mput(s8, '0');

		if(mget(s1) && mget(s2) && mget(s3) && mget(s4))
			break;
	}

	mput(s5, '0');
	mput(s6, '0');
	mput(s7, '0');
	mput(s8, '0');

	mclose(s1);
	mclose(s2);
	mclose(s3);
	mclose(s4);
	mclose(s5);
	mclose(s6);
	mclose(s7);
	mclose(s8);
	mdelete("ts1");
	mdelete("ts2");
	mdelete("ts3");
	mdelete("ts4");
	mdelete("orange");
	mdelete("yellow");
	mdelete("green");
	mdelete("blue");

	return 0;
}

/*3. pot2ser: Continuously output the value of the analog potentiomemter to the serial device as a decimal
            or hexadecimal number followed by a newline.

    End on a value of zero.
*/
//sw2 used as an emergency exit from command
int cmd_pot2ser(int argc, char *argv[]){
	//Check arguments
	if(argc > 2){
		return 1;
	}

	unsigned int out = 1, format;
	char *fout = malloc(15);

	if(argc == 1)
		format = 0;
	else if(compare_string(argv[2], "dec"))
		format = 0;
	else if(compare_string(argv[2], "hex"))
		format = 1;
	else
		return ERR_CMD_FAIL;

	mcreate("analog");
	stream* s1 = mopen("analog", "r");

	while(out && !sw2In()) {

		delay(115200);

		out = mget(s1);

		if(format == 0) {
			sprintf(fout, "%d\r\n", out);
			uartPuts(UART2_BASE_PTR, fout);
		}
		else{
			longInt2hex((unsigned long int)out, fout);
			sprintf(fout, "%s\r\n", fout);
			uartPuts(UART2_BASE_PTR, fout);;
		}
	}

	mclose(s1);
	mdelete("analog");
	free(fout);
	return 0;
}


/*4. therm2ser: Continuously output the value of the thermistor to the serial device as a decimal or hexadecimal
                number followed by a newline.

	End when SW1 is depressed.
*/
//sw2 button used as an emergency exit from function
int cmd_therm2ser(int argc, char *argv[]){
	//Check arguments
	if(argc > 2){
		return 1;
	}

	unsigned int out = 1, format;
	char *fout = malloc(15);

	if(argc == 1)
		format = 0;
	else if(compare_string(argv[2], "dec"))
		format = 0;
	else if(compare_string(argv[2], "hex"))
		format = 1;
	else
		return ERR_CMD_FAIL;

	mcreate("thermistor");
	stream* s1 = mopen("thermistor", "r");

	while(out && !sw2In()) {

		delay(115200);

		out = mget(s1);

		if(format == 0) {
			sprintf(fout, "%d\r\n", out);
			uartPuts(UART2_BASE_PTR, fout);
		}
		else{
			longInt2hex((unsigned long int)out, fout);
			sprintf(fout, "%s\r\n", fout);
			uartPuts(UART2_BASE_PTR, fout);;
		}
	}

	mclose(s1);
	mdelete("thermistor");
	free(fout);
	return 0;
}

//5. pb2led: Continuously copy from SW1 to orange LED and SW2 to yellow LED.
//        End when both SW1 and SW2 are depressed.
int cmd_pb2led(int argc, char *argv[]){
	//Check arguments
	if(argc > 1){
		return 1;
	}

	mcreate("sw1");
	mcreate("sw2");
	mcreate("orange");
	mcreate("yellow");
	stream* s1 = mopen("sw1", "r");
	stream* s2 = mopen("sw2", "r");
	stream* s3 = mopen("orange", "w");
	stream* s4 = mopen("yellow", "w");

	while(1) {

		delay(60000);

		if(mget(s1))
			mput(s3, '1');
		else
			mput(s3, '0');

		if(mget(s2))
			mput(s4, '1');
		else
			mput(s4, '0');

		if(mget(s1) && mget(s2)) {
			mput(s3, '0');
			mput(s4, '0');
			break;
		}
	}

	mclose(s1);
	mclose(s2);
	mclose(s3);
	mclose(s4);
	mdelete("sw1");
	mdelete("sw2");
	mdelete("orange");
	mdelete("yellow");
	return 0;
}



/* ***********************************              HELPER FUNCTIONS            ****************************
   *********************************************************************************************************  */

//return the pointer to a stream from the users input, return NULL on failure
stream *get_strm(char *argv[], char *type) {

	stream *strm = NULL;
	char *desc = argv[1];

	*type = desc[0];
	desc = &(desc[1]);

	if(*type == 'f') {
		int fd = string_to_num(desc);

		if(fd < 0) return strm;
		strm = open_f[fd];
	}
	else if(*type == 'd') {
		int dd = string_to_num(desc);

		if(dd < 0) return strm;
		strm = open_d[dd];
	}

	return strm;
}

//function for converting arguments into integers(positives only)
int string_to_num(char *p) {

	int i = 0;
	int add = 0;
	int num = 0;
	char *nums = ("0123456789");

	while(p[i] != '\0') { i++; }

	if( i < 1) return -1;
	int temp = i;

	for(int j = 0; j < temp; j++){
		for(int k = 0; k < 11; k++) {
			if(k == 10)
				return -1;
			else if(p[i-1] == nums[k])
				break;
		}

		add = p[i-1] - '0';           //convert char to integer
		add = add * power(10, j);
		num = num + add;
		i--;
	}

	return num;
}

//exponent function for  'string_to_num', raises @base to @exp
int power(int base, unsigned int exp) {
	int i, result = 1;
	for (i = 0; i < exp; i++)
		result *= base;
	return result;
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
