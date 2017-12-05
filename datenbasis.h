#if !defined _DATENBASIS_H_
#define _DATENBASIS_H_

#include "uthash.h"

#include <net/if.h>
#include <linux/can.h>
#include "lib.h"


struct signal_struct {
	char name[80];
	int startBit;
	int signalLength;
	int is_big_endian; // Intel = 0; Motorola (== BIG Endian) = 1
	int is_signed;
	float factor;
	float offset;
	float min;
	float max;
	char unit[80];
	char receiverList[512];
        __u8 isMultiplexer;
        __u8 muxId;
        
	__u8 number;
	UT_hash_handle hh;
};

struct frame_struct {
	canid_t canID;
	char name[80];
        __u8 isMultiplexed;
	struct signal_struct *signals;
	UT_hash_handle hh;
};


#endif


void add_message(struct frame_struct **db, canid_t canID, char *messageName);
struct frame_struct *find_frame(struct frame_struct *db, int canID);
struct frame_struct *find_frame_by_name(struct frame_struct *db, char *name);
struct signal_struct *find_signal_by_name(struct frame_struct *frame, char *name);
struct frame_struct *find_frame_by_signalname(struct frame_struct *db, char *name);
void add_signal(struct frame_struct *db, int frameId, char *signalName, int startBit, int signalLength, int byteOrder, int signedState, float factor, float offset, float min, float max, char *unit, char *receiverList, unsigned char isMultiplexer, unsigned char muxId);
int readInDatabase(struct frame_struct **db, char *Filename);

