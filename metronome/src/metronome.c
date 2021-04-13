struct ioattr_t;
#define IOFUNC_ATTR_T struct ioattr_t
struct metro_ocb;
#define IOFUNC_OCB_T struct metro_ocb

#include <sys/iofunc.h>
#include <unistd.h>
#include <errno.h>
#include <sys/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define METRONOME_PULSE_CODE _PULSE_CODE_MINAVAIL
#define START_PULSE_CODE 1
#define STOP_PULSE_CODE 2
#define PAUSE_PULSE_CODE 3
#define SET_PULSE_CODE 4
#define QUIT_PULSE_CODE 5

#define START 0
#define PAUSED 1
#define STOPPED 2
#define ERROR (-1)

#define ATTACH_METRONOME  "metronome"

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

struct DataTableRow {
	int tst;
	int tsb;
	int intervals;
	char pattern[20];
};

struct DataTableRow t[] = { { 2, 4, 4, "|1&2&" }, { 3, 4, 6, "|1&2&3&" }, { 4,
		4, 8, "|1&2&3&4&" }, { 5, 4, 10, "|1&2&3&4-5-" },
		{ 3, 8, 6, "|1-2-3-" }, { 6, 8, 6, "|1&a2&a" },
		{ 9, 8, 9, "|1&a2&a3&a" }, { 12, 8, 12, "|1&a2&a3&a4&a" } };

struct Timer_Setting {
	double min_per_bpm;
	double nano_sec;
	double measure;
	double interval;
}typedef timer_setting_t;

struct Metronome_Property {
	int tst;
	int tsb;
	int bpm;
}typedef metronome_property;

struct Metronome {
	metronome_property metronome_props;
}typedef Metronome_t;

#define DEVICES 2
#define METRONOME 0
#define METRONOME_HELP 1

char *metro_devices[DEVICES] = { "/dev/local/metronome",
		"/dev/local/metronome-help" };

typedef struct ioattr_t {
	iofunc_attr_t attr;
	int device;
} ioattr_t;

typedef struct metro_ocb {
	iofunc_ocb_t ocb;
	char buffer[50];
} metro_ocb_t;

void *metronome_thread();
void check_for_device(metro_ocb_t *mocb);
int get_table_index(Metronome_t * Metronome);
void set_timer_start(struct itimerspec * itime, timer_t timer_id,
		Metronome_t* Metronome);
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra);
metro_ocb_t * metronome_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *mtattr);
void metronome_ocb_free(IOFUNC_OCB_T *mocb);
void write_pause_pulse(char *pause_input, int small_integer, char *buf);
void set_metronome_props(char *buf, char * set_input);
void invalidInput(char *buf);
void set_event(struct sigevent *event);

name_attach_t * metronome_attach;
Metronome_t metronome;
timer_setting_t timer_set;
int server_coid;
char data[255];
pthread_attr_t thread_attrib;

int main(int argc, char *argv[]) {
	dispatch_t * dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t conn_funcs;
	ioattr_t ioattr[DEVICES];
	dispatch_context_t * ctp;
	int server_coid;

	if (argc != 4) { // checking for invalid number of arguments
		printf("ERROR: Invalid number of argument entered.\n");
		printf(
				"Usage: metronome beats-per-minute time-signature-top time-signature-bottom\n");
		exit(EXIT_FAILURE);
	}

	iofunc_funcs_t metronome_ocb_funcs = { // define which functions to call when doing memory resizing
			_IOFUNC_NFUNCS, metronome_ocb_calloc, metronome_ocb_free, };
	iofunc_mount_t metro_mount = { 0, 0, 0, 0, &metronome_ocb_funcs };

	//assigning metronome setting from the arguments
	metronome.metronome_props.bpm = atoi(argv[1]);
	metronome.metronome_props.tst = atoi(argv[2]);
	metronome.metronome_props.tsb = atoi(argv[3]);

	if ((dpp = dispatch_create()) == NULL) { //creating dispatch
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		return (EXIT_FAILURE);
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &conn_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);

	//override connect and io functions
	conn_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	int id, i;

	for (i = 0; i < DEVICES; i++) {

		if (i == 0) { // Initialize the attribute structure associated with a device
			iofunc_attr_init(&ioattr[i].attr, S_IFCHR | 0666, NULL, NULL);
		} else {
			iofunc_attr_init(&ioattr[i].attr, S_IFCHR | 0444, NULL, NULL);
		}

		ioattr[i].device = i;
		ioattr[i].attr.mount = &metro_mount;

		//registering the device with resource manager
		if ((id = resmgr_attach(dpp, NULL, metro_devices[i], _FTYPE_ANY, 0,
				&conn_funcs, &io_funcs, &ioattr[i])) == ERROR) {
			fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
			return (EXIT_FAILURE);
		}
	}

	ctp = dispatch_context_alloc(dpp);

	pthread_attr_init(&thread_attrib); //initializes the thread attributes
	pthread_create(NULL, &thread_attrib, &metronome_thread, NULL); //starts a new thread in the calling process.

	while (1) {
		if ((ctp = dispatch_block(ctp))) { //blocks while waiting for an event
			dispatch_handler(ctp);
		} else
			printf("ERROR \n");
	}

	pthread_attr_destroy(&thread_attrib);
	name_detach(metronome_attach, 0);
	name_close(server_coid);
	return EXIT_SUCCESS;
}

void check_for_device(metro_ocb_t *mocb) {
	int index;

	if (mocb->ocb.attr->device == METRONOME_HELP) { // if the device called is metronome-help, it will print out the metronome api information
		sprintf(data,
				"Metronome Resource Manager (Resmgr)\n\n Usage: metronome <bpm> <ts-top> <ts-bottom>\n\n API:\n  pause[1-9]\t\t\t- pause the metronome for 1-9 seconds\n  quit:\t\t\t\t- quit the metronome\n  set<bpm><ts-top><ts-bottom>\t- set the metronome to <bpm> ts-top/ts-bottom\n  start\t\t\t\t- start the metronome from stopped state\n  stop\t\t\t\t- stop the metronome; use 'start' to resume\n");
	} else {
		index = get_table_index(&metronome);
		sprintf(data,
				"[metronome: %d beats/min, time signature %d/%d, secs-per-interval: %.2f, nanoSecs: %.0lf]\n",
				metronome.metronome_props.bpm, t[index].tst, t[index].tsb,
				timer_set.interval, timer_set.nano_sec);
	}
}

// reading data from the resource
int io_read(resmgr_context_t *ctp, io_read_t *msg, metro_ocb_t *mocb) {

	int nb;

	if (data == NULL)
		return 0;

	check_for_device(mocb);

	nb = strlen(data);

	if (mocb->ocb.offset == nb)
		return 0;

	nb = min(nb, msg->i.nbytes);

	_IO_SET_READ_NBYTES(ctp, nb);

	SETIOV(ctp->iov, data, nb);

	mocb->ocb.offset += nb;

	if (nb > 0)
		mocb->ocb.flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

// implements pause api
void write_pause_pulse(char *pause_input, int small_integer, char *buf) {
	int i = 0;

	for (i = 0; i < 2; i++) {
		pause_input = strsep(&buf, " ");
	}

	small_integer = atoi(pause_input);

	if (small_integer >= 1 && small_integer <= 9) {
		MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
		PAUSE_PULSE_CODE, small_integer);
	} else {
		printf("Integer is not between 1 and 9.\n");
	}
}

// sets metronome properties
void set_metronome_props(char *buf, char * set_input) {
	int i = 0;

	for (i = 0; i < 4; i++) {
		set_input = strsep(&buf, " ");

		switch (i) {
		case 1:
			metronome.metronome_props.bpm = atoi(set_input);
			break;
		case 2:
			metronome.metronome_props.tst = atoi(set_input);
			break;
		case 3:
			metronome.metronome_props.tsb = atoi(set_input);
			break;
		}
	}
}

// prints a message for an invalid input
void invalidInput(char *buf) {
	char input[] = "\n";
	char *invalidMsg;

	invalidMsg = strtok(buf, input);

	fprintf(stderr, "\nError - '%s' is not a valid command\n", invalidMsg);
	strcpy(data, buf);
}

//responsible for writing the data into the device
int io_write(resmgr_context_t *ctp, io_write_t *msg, metro_ocb_t *mocb) {

	int nb = 0;

	if (mocb->ocb.attr->device == METRONOME_HELP) {
		printf("\nUnable to write to device /dev/local/metronome-help\n");
		nb = msg->i.nbytes;
		_IO_SET_WRITE_NBYTES(ctp, nb);
		return (_RESMGR_NPARTS(0));
	}

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		char *buf;
		char * pause_input;
		char * set_input;
		int small_integer = 0;
		buf = (char *) (msg + 1);

		if (strstr(buf, "start") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), START_PULSE_CODE,
					small_integer);
		} else if (strstr(buf, "stop") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), STOP_PULSE_CODE,
					small_integer);
		} else if (strstr(buf, "quit") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), QUIT_PULSE_CODE,
					small_integer);
		} else if (strstr(buf, "pause") != NULL) {
			pause_input = NULL;
			write_pause_pulse(pause_input, small_integer, buf);
		} else if (strstr(buf, "set") != NULL) {
			set_input = NULL;
			set_metronome_props(buf, set_input);
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL), SET_PULSE_CODE,
					small_integer);
		} else {
			invalidInput(buf);
		}

		nb = msg->i.nbytes;
	}

	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		mocb->ocb.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

// opens the registered device
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {

	if ((server_coid = name_open(ATTACH_METRONOME, 0)) == ERROR) {
		perror("ERROR - name_open failed - io_open() \n ");
		return EXIT_FAILURE;
	}

	return (iofunc_open_default(ctp, msg, &handle->attr, extra));
}

//set event properites
void set_event(struct sigevent *event) {
	event->sigev_notify = SIGEV_PULSE;
	event->sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, metronome_attach->chid,
	_NTO_SIDE_CHANNEL, 0);
	event->sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
	event->sigev_code = METRONOME_PULSE_CODE;
}

// connects with the registered devices and implements appropriate functions from the pulse code
void *metronome_thread() {
	struct sigevent event;
	struct itimerspec itime;
	timer_t timer_id;
	my_message_t msg;
	int rcvid;
	int tableIndex = 0;
	char * metronome_pattern;
	int status;

	if ((metronome_attach = name_attach(NULL, ATTACH_METRONOME, 0)) == NULL) {
		printf("\nUnable to connect to the device\n");
		exit(EXIT_FAILURE);
	}

	set_event(&event);
	timer_create(CLOCK_REALTIME, &event, &timer_id);
	tableIndex = get_table_index(&metronome);

	timer_set.min_per_bpm = (double) 60 / metronome.metronome_props.bpm;
	timer_set.measure = timer_set.min_per_bpm * metronome.metronome_props.tst;
	timer_set.interval = timer_set.measure / t[tableIndex].intervals;
	timer_set.nano_sec = (timer_set.interval - (int) timer_set.interval) * 1e+9;

	set_timer_start(&itime, timer_id, &metronome);
	metronome_pattern = t[tableIndex].pattern;

	for (;;) {
		rcvid = MsgReceive(metronome_attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == ERROR) {
			printf("Unable to receive message\n");
			exit(EXIT_FAILURE);
		}

		switch (msg.pulse.code) {

		case METRONOME_PULSE_CODE:

			if (*metronome_pattern == '|') {
				printf("%.2s", metronome_pattern);
				metronome_pattern = (metronome_pattern + 2);
			} else if (*metronome_pattern != '\0') {
				printf("%c", *metronome_pattern++);
			} else if (*metronome_pattern == '\0') {
				printf("\n");
				metronome_pattern = t[tableIndex].pattern;
			}

			break;
		case START_PULSE_CODE:
			tableIndex = get_table_index(&metronome);
			metronome_pattern = t[tableIndex].pattern;
			set_timer_start(&itime, timer_id, &metronome);
			status = START;
			printf("\n");
			break;
		case STOP_PULSE_CODE:

			if (status != STOPPED) {
				itime.it_value.tv_sec = 0;
				timer_settime(timer_id, 0, &itime, NULL);
				status = STOPPED;
			}

			break;

		case PAUSE_PULSE_CODE:
			itime.it_value.tv_sec = msg.pulse.value.sival_int;
			timer_settime(timer_id, 0, &itime, NULL);
			break;
		case SET_PULSE_CODE:
			tableIndex = get_table_index(&metronome);
			metronome_pattern = t[tableIndex].pattern;

			timer_set.min_per_bpm = (double) 60 / metronome.metronome_props.bpm;
			timer_set.measure = timer_set.min_per_bpm
					* metronome.metronome_props.tst;
			timer_set.interval = timer_set.measure / t[tableIndex].intervals;
			timer_set.nano_sec = (timer_set.interval - (int) timer_set.interval)
					* 1e+9;

			set_timer_start(&itime, timer_id, &metronome);

			printf("\n");
			break;

		case QUIT_PULSE_CODE:
			timer_delete(timer_id);
			name_detach(metronome_attach, 0);
			name_close(server_coid);
			pthread_attr_destroy(&thread_attrib);
			exit(EXIT_SUCCESS);
		}

		fflush(stdout);
	}

	return NULL;
}

//finds an index from the metronome data table
int get_table_index(Metronome_t * Metronome) {
	int i = 0;

	for (i = 0; i < 8; i++) {
		if (t[i].tsb == Metronome->metronome_props.tsb
				&& t[i].tst == Metronome->metronome_props.tst) {
			return i;
		}
	}

	return ERROR;
}

// set the timer to start
void set_timer_start(struct itimerspec * itime, timer_t timer_id,
		Metronome_t* Metronome) {
	itime->it_value.tv_sec = 1;
	itime->it_value.tv_nsec = 0;
	itime->it_interval.tv_sec = timer_set.interval;
	itime->it_interval.tv_nsec = timer_set.nano_sec;

	timer_settime(timer_id, 0, itime, NULL);
}

metro_ocb_t * metronome_ocb_calloc(resmgr_context_t *ctp, ioattr_t *mattr) {
	metro_ocb_t *metro_ocb;
	metro_ocb = calloc(1, sizeof(metro_ocb_t));
	metro_ocb->ocb.offset = 0;
	return (metro_ocb);
}

void metronome_ocb_free(metro_ocb_t *mocb) {
	free(mocb);
}

