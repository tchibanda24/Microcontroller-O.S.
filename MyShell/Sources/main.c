/*
 * main.c
 *
 *  Main function that runs the shell program for the user
 *
 *  Created on: Nov 1, 2015
 *      Author: Thabani Chibanda
 */

#include "myShell.h"
#include "derivative.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

//helper
static int get_line(char *line);
static int fill_argv(char *argv[], char *line, int line_len);
void copy_string(char *d, char *s);

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

static const char *ERROR_STRINGS[] = {
		"ERR: Invalid command. Only: date, echo, malloc, free, mmaphelp, help, exit.\r\n",
		"ERR: Incorrect amount of argumets for that command. Enter 'help' to learn more about the commands.\r\n",
		"ERR: Command failed.\r\n",
		"ERR: Argument must be one of the built in commands(malloc, free, mmap, create, delete, open, close, read, write, seek, date, echo, exit, help).\r\n",
		"ERR: File or device not found.\r\n",
		"ERR: File in use.\r\n",
		"ERR: Could not create file.\r\n",
		"ERR: Can't open file.\r\n",
		"ERR: Read failed. End of file.\r\n",
};

//original command structure, currently needed both here and in my shell until externalized
struct commandEntry {
	char *name;
	int (*functionp)(int argc, char *argv[]);
	char *desc;
}  commands[] = {{"create", cmd_create, "help: Accepts one argument, a file name(can only contain numbers, letters, underscore, dash, and accepts any extension). A file in memory will be created with that name.\r\n"},
		{"delete", cmd_delete, "help: Accepts one argument, the name of an existing file in memory. Will delete the file only if it is not currently opened. File cannot be accesed after being deleted.\n"},
		{"open", cmd_open, "help: Accepts two arguments, the name of an existing file/device('f/filename.txt' - 'd/devicename') in memory and how you would like that file/device to be opened(r = read, w = write, rw = read/write) : devicenames: 'sw1'-'sw2'-'orange'-'yellow'-'green'-'blue'.\n-Files will be accessed from the beginning, so any data currently in the file will be overwritten if opened for writing.\n-Only one writer allowed at one time, but multiple readers are allowed. A file can not be written to while being read, and vice versa.\n-Will return a file descriptor for handling files.\r\n"},
		{"close", cmd_close, "help: Accepts one argument, the name of an existing file in memory. Will close the file only for the stream passed to it.\r\n"},
		{"read", cmd_read, "help: Accepts one argument, a file descripter or device name. Displays either the current character pointed to in a file(and slides that pointer down) or the current state of a device.\r\n"},
		{"write", cmd_write, "help: Accepts two argumentsa, file/device  descripter along with a single character/integer for writing.\n-For devices: '0' = off, '1' = on.\r\n"},
		{"seek", cmd_seek, "help: Accepts two arguments, the name of an existing file in memory and a position in that file to be read or written to. '-1' can be entered to be put at the end of the file.\r\n"},
		{"ls", cmd_list, "help: Accepts no arguments. Returns the name of files in directory.\r\n"},
		{"malloc", cmd_malloc, "help: Accepts one integer argument. Allocates memory for amount specified, and returns address of the memory.\r\n"},
		{"free", cmd_free, "help: Accepts one argument. Frees a previously allocated block of memory at the address given.\r\n"},
		{"mmap", cmd_mmap, "date: Accepts no arguments. Outputs the current addresses allocated by the shell.\r\n"},
		{"echo", SVCEcho, "echo: Accepts any amount of arguments. Outputs, or \"echoes\", the arguments you enter along with it.\r\n"},
		{"date", cmd_date, "date: Accepts no arguments. Outputs the current date and time.\n"},
		{"exit", cmd_exit, "exit: Accepts no arguments. Quits the shell program.\r\n"},
		{"help", cmd_help, "help: Accepts one or no arguments. Gives descriptions of the available commands.\r\n"},
		{"ser2lcd", cmd_ser2lcd, "ser2lcd: Accepts no arguments. Continuously copies characters from input to LCD.  Ends on a ^D (control-D) input character.\r\n"},
		{"touch2led", cmd_touch2led, "touch2led: Accepts no arguments. Continuously copies from each touch sensor to the corresponding LED.  Stops when all four touch sensors are  depressed.\r\n"},
		{"pot2ser", cmd_pot2ser, "pot2ser: Accepts one or no arguments. Continuously outputs the value of the analog potentiomemter to the serial device as a decimal(with no arguments or 'dec' as an argument) or hexadecimal(with 'hex' as an argument) number followed by a newline.\r\n"},
		{"therm2ser", cmd_therm2ser, "therm2ser: Accepts one or no arguments. Continuously outputs the value of the analog thermistor device to the serial device as a decimal(with no arguments or 'dec' as an argument) or hexadecimal(with 'hex' as an argument) number followed by a newline.\r\n"},
		{"pb2led", cmd_pb2led, "pb2led: Accepts no arguments. Continuously copies from SW1 to orange LED and SW2 to  yellow LED.  Stops when both SW1 and SW2 are depressed.\r\n"},
		{"intTest1", cmd_intTest1, "intTest1: copies from UART2 input to the LCD display using device independent supervisor calls.\n"},
		{"intTest2", cmd_intTest2, "intTest2: sends a message over UART2 output whenever pushbutton S2 is depressed.\n"},
		{"intTest3", cmd_intTest3, "intTest3: uses the supervisor call for user timer events, flash the orange LED on and off every half a second (the LED will light once a second).\n"},
};

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

int main() {

	const int moduleClock = FLL_Factor * IRC;
	shell_Init(moduleClock);
	//privUnprivileged();

	int argc = 0, exit = 0, line_len = 0, cmd_entered = -1, cmd_code = 0;
	char line[LINE_MAX], *argv[LINE_MAX];

	for(int i = 0; i < MAX_STREAMS; i++) open_f[i] = NULL;
	for(int i = 0; i < MAX_STREAMS; i++) open_d[i] = NULL;

	while(!exit) {
		for(int i = 0; i < LINE_MAX; i++) line[i] = '\0';
		for(int i = 0; i < LINE_MAX; i++) argv[i] = NULL;

		line_len = 0;
		cmd_code = 0;
		cmd_entered = -1;

		//Retrieve input
		while(line_len == 0) {
			putsIntoBuffer("Shell:~ TC$ ");
			lcdcConsolePuts(&console, "Shell:~ TC$ ");
			line_len = get_line(line);
		}

		argc = fill_argv(argv, line, line_len);             //seperate arguments

		//Determine the command entered and perform it, error message if invalid
		for(int i = 0; i < NUM_CMDS; i++){
			if(strcmp(argv[0], commands[i].name) == 0)
				cmd_entered = i;
		}

		if(cmd_entered != -1){
			cmd_code = commands[cmd_entered].functionp(argc, argv);

			//Evaluate code returned by commmand
			if(cmd_code != 0) {
				putsIntoBuffer((char*)ERROR_STRINGS[cmd_code]);
				lcdcConsolePuts(&console, (char*)ERROR_STRINGS[cmd_code]);
			}
			//if(cmd_code != 0)
				//uartPuts(UART2_BASE_PTR,  (char*)ERROR_STRINGS[cmd_code]);
		}
		else {
			putsIntoBuffer((char*)ERROR_STRINGS[cmd_code]);
			lcdcConsolePuts(&console, (char*)ERROR_STRINGS[cmd_code]);
			//uartPuts(UART2_BASE_PTR, (char*)ERROR_STRINGS[ERR_INVALID_CMD]);
		}

		for(int i = 0; i < argc; i++)
			free(argv[i]);
	}
	cmd_exit(argc, argv);
}

//Reads in the line inputed by the user, returns the length of the line
int get_line(char *line){

	int ch, length = 0;

	//loop that runs until it meets the newline character
	while(1){

		ch = getcharFromBuffer();
		putcharIntoBuffer(ch);
		lcdcConsolePutc(&console, ch);

		while((s2OutCt > 0) || !(UART2_S1 & UART_S1_TC_MASK)) {
		}

		if (ch == '\r') {
			line[length] = '\0';
			putcharIntoBuffer('\n');
			lcdcConsolePutc(&console, '\n');

			while((s2OutCt > 0) || !(UART2_S1 & UART_S1_TC_MASK)) {
			}

			break;
		}
		else {
			line[length] = (char)ch;
		}

		length++;
	}

	return length;
}

//Initializes the @argv with the arguments from the user, returns the number of arguemnts
int fill_argv(char *argv[], char *line, int line_len){

	int argc = 0, i = 0;
	char word[LINE_MAX];
	char ch;

	for(int i = 0; i < LINE_MAX; i++) word[i] = '\0';           //initilize @word
	while((argc == 0) && (line[i] == '\0')) i++;                 //ignore leading spaces

	//runs a loop that seperates the line into separate arguments(separated by spaces)
	for(i; i < line_len; i++){
		int j = 0;
		for(int k = 0; k < LINE_MAX; k++) word[k] = '\0';

		while(line[i] == '\0') i++;

		if(i == line_len)
			break;

		while((line[i] != ' ') && (line[i] != '\0')){

			ch = (char)(line[i]);
			word[j] = ch;

			j++; i++;
		}

		argv[argc] = malloc(j + 1);
		copy_string(argv[argc], word);
		argc++;
	}

	return argc;
}
