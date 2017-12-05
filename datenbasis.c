#include <stdio.h>
#include "datenbasis.h"


void add_frame(struct frame_struct **db, canid_t canID, char *frameName) {
	struct frame_struct *s;
	s = malloc(sizeof(struct frame_struct));
	s->canID = canID;
	s->signals = NULL;
	strcpy(s->name, frameName);
        s->isMultiplexed = 0;   
	HASH_ADD_INT( *db, canID, s );
}

struct frame_struct *find_frame(struct frame_struct *db, int canID) {
	struct frame_struct *s;
	HASH_FIND_INT( db, &canID, s );
	return s;
	/* s: output pointer */
}

struct frame_struct *find_frame_by_name(struct frame_struct *db, char *name) {
	struct frame_struct *s;

	for(s=db; s != NULL; s=s->hh.next) {
		if(!strcmp(s->name, name)) return s;
        }

	return 0;
	/* s: output pointer */
}

struct signal_struct *find_signal_by_name(struct frame_struct *frame, char *name) {
	struct signal_struct *sig;

	for(sig=frame->signals; sig != NULL; sig = sig->hh.next) {
			if(!strcmp(sig->name, name)) return sig;
    		}

	return 0;
}

struct frame_struct *find_frame_by_signalname(struct frame_struct *db, char *name) {
	struct frame_struct *frame;
	struct signal_struct *sig;

	for(frame=db; frame != NULL; frame=frame->hh.next) {
		for(sig=frame->signals; sig != NULL; sig = sig->hh.next) {
			if(!strcmp(sig->name, name)) return frame;
    		}
        }

	return 0;
	/* s: output pointer */
}

void add_signal(struct frame_struct *db, int frameId, char *signalName, int startBit, int signalLength, int is_big_endian, int signedState, float factor, float offset, float min, float max, char *unit, char *receiverList, unsigned char isMultiplexer, unsigned char muxId)
{
	struct frame_struct *frame;
	struct signal_struct *newSignal;

	frame = find_frame(db, frameId);
	
	newSignal = malloc(sizeof(struct signal_struct));
	strcpy(newSignal->name, signalName);
	newSignal->startBit = startBit;
	newSignal->signalLength = signalLength;
	newSignal->is_big_endian = is_big_endian;
	newSignal->is_signed = signedState;
	newSignal->factor = factor;
	newSignal->offset = offset;
	newSignal->min = min;
	newSignal->max = max;
        
        
        if(isMultiplexer > 0)
        {
            frame->isMultiplexed = 1;
        }
        newSignal->isMultiplexer = isMultiplexer;
        newSignal->muxId = muxId;
	
	strcpy(newSignal->unit, unit);
	strcpy(newSignal->receiverList, receiverList);

	HASH_ADD_STR( frame->signals, name, newSignal );
}



int readInDatabase(struct frame_struct **db, char *Filename)
{
	char frameName[512], sender[512], line[512];
	char signalName[512],signedState, unit[512], receiverList[512];
	int startBit=0,signalLength=0,byteOrder=0;
	float factor=0., offset=0., min=0., max=0.;
        char mux[4];
        int muxId = 0;

	int frameId=0, len;

	FILE *fp;
	if(!(fp = fopen(Filename,"r")))
	{
		fprintf(stderr, "Error opening  %s\n", Filename);
		return 1;
	}

	while(fgets(line,511,fp))
	{
		if(sscanf(line," BO_ %d %s %d %s",&frameId,frameName,&len,sender) == 4)
		{
			frameName[strlen(frameName)-1] = 0;			
			add_frame(db, frameId, frameName);
		}
		else if(sscanf(line," SG_ %s : %d|%d@%d%c (%f,%f) [%f|%f] %s %s",signalName, &startBit, &signalLength,&byteOrder, &signedState, &factor, &offset, &min, &max, unit, receiverList ) > 5)
		{
                    if (byteOrder == 0)
                    {
                        // following code is from https://github.com/julietkilo/CANBabel/blob/master/src/main/java/com/github/canbabel/canio/dbc/DbcReader.java:

                        int pos = 7 - (startBit % 8) + (signalLength - 1);
                        if (pos < 8)
                        {
                            startBit = startBit - signalLength + 1;
                        }
                        else
                        {
                            int cpos = 7 - (pos % 8);
                            int bytes = (int)(pos / 8);
                            startBit = cpos + (bytes * 8) + (int)(startBit/8) * 8;
                        }
                    }
                    add_signal(*db, frameId, signalName, startBit, signalLength, byteOrder == 0, signedState == '-', factor, offset, min, max, unit, receiverList, 0,0);		
		}
		else if(sscanf(line," SG_ %s %s : %d|%d@%d%c (%f,%f) [%f|%f] %s %s",signalName, mux, &startBit, &signalLength,&byteOrder, &signedState, &factor, &offset, &min, &max, unit, receiverList ) > 5)
		{
                    if (byteOrder == 0)
                    {
                        // following code is from https://github.com/julietkilo/CANBabel/blob/master/src/main/java/com/github/canbabel/canio/dbc/DbcReader.java:

                        int pos = 7 - (startBit % 8) + (signalLength - 1);
                        if (pos < 8)
                        {
                            startBit = startBit - signalLength + 1;
                        }
                        else
                        {
                            int cpos = 7 - (pos % 8);
                            int bytes = (int)(pos / 8);
                            startBit = cpos + (bytes * 8) + (int)(startBit/8) * 8;
                        }
                    }
                    if(mux[0] == 'M')
                    {
                        add_signal(*db, frameId, signalName, startBit, signalLength, byteOrder == 0, signedState == '-', factor, offset, min, max, unit, receiverList, 1, 0);
                    }
                    else if(mux[0] == 'm')
                    {
                        sscanf(mux, "m%d", &muxId);
                        add_signal(*db, frameId, signalName, startBit, signalLength, byteOrder == 0, signedState == '-', factor, offset, min, max, unit, receiverList, 2, muxId);
                        
                    }
                    
		}
	}
	return 0;
}
