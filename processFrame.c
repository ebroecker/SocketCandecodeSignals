#include <stdio.h>
#include "processFrame.h"
#include "stdbool.h"
#include "can_encode_decode_inl.h"

float toPhysicalValue(uint64_t target, float factor, float offset,
bool is_signed);
uint64_t extractSignal(const uint8_t* frame, const uint8_t startbit, const uint8_t length, bool is_big_endian, bool is_signed);


void add_callback(struct signal_callback_list **callbackList, struct frame_struct *frame, struct signal_struct *signal,
		void (*callback)(char *, __u64, double, struct timeval, char *device), __u8 onChange)
{

	struct signal_callback_list *callbackItem;

	callbackItem = malloc(sizeof(struct signal_callback_list));
	callbackItem->frame = frame;
	callbackItem->signal = signal;
	callbackItem->callback = callback;
	callbackItem->rawValue = 0;
	callbackItem->onChange = onChange;

	HASH_ADD_INT(*callbackList, signal, callbackItem);
}

void processFrame(struct signal_callback_list *callbackList, struct can_frame *cf, struct timeval tv, char *device)
{
	struct signal_callback_list *callbackItem;
	struct signal_struct *signal;
	__u64 value = 0;
	double scaled = 0.;
	unsigned int muxerVal = 0;
	bool muxerFound = false;

	for (callbackItem = callbackList; callbackItem != NULL; callbackItem = callbackItem->hh.next)
	{
		if (callbackItem->frame->canID != cf->can_id)
			continue;

		if (callbackItem->frame->isMultiplexed)
		{
			// find multiplexer and report its value:
			for (signal = callbackItem->frame->signals; signal != NULL; signal = signal->hh.next)
			{
				if (1 == signal->isMultiplexer)
				{
					value = extractSignal(cf->data, signal->startBit, signal->signalLength, (bool) signal->is_big_endian, signal->is_signed);
					scaled = toPhysicalValue(value, signal->factor, signal->offset, signal->is_signed);
					muxerVal = (unsigned int) value;
					muxerFound = true;
					(callbackItem->callback)(signal->name, value, scaled, tv, device);
					break;
				}
			}
		}

		if (callbackItem->signal == NULL)
		{
			// whole frame subscribed: report all signals
			for (signal = callbackItem->frame->signals; signal != NULL; signal = signal->hh.next)
			{
				if (1 == signal->isMultiplexer)
					continue; // already reported above
				// in multiplexed frames only decode plain signals and signals
				// of the active multiplexer group
				if (callbackItem->frame->isMultiplexed && 2 == signal->isMultiplexer
						&& (!muxerFound || signal->muxId != muxerVal))
					continue;

				value = extractSignal(cf->data, signal->startBit, signal->signalLength, (bool) signal->is_big_endian, signal->is_signed);
				if (callbackItem->onChange && callbackItem->rawValue == value)
					continue;
				callbackItem->rawValue = value;
				scaled = toPhysicalValue(value, signal->factor, signal->offset, signal->is_signed);
				(callbackItem->callback)(signal->name, value, scaled, tv, device);
			}
		}
		else
		{
			signal = callbackItem->signal;
			if (callbackItem->frame->isMultiplexed
					&& (1 == signal->isMultiplexer
							|| (2 == signal->isMultiplexer
									&& (!muxerFound || signal->muxId != muxerVal))))
				continue; // muxer already reported, or signal not in this mux group

			value = extractSignal(cf->data, signal->startBit, signal->signalLength, (bool) signal->is_big_endian, signal->is_signed);
			if (callbackItem->onChange && callbackItem->rawValue == value)
				continue;
			callbackItem->rawValue = value;
			scaled = toPhysicalValue(value, signal->factor, signal->offset, signal->is_signed);
			(callbackItem->callback)(signal->name, value, scaled, tv, device);
		}
	}
}

void releaseSignalCallbackResource(struct signal_callback_list **cb_list)
{
  struct signal_callback_list *signal_cb;
  for (signal_cb = *cb_list; signal_cb != NULL;)
  {
    struct signal_callback_list *signal_cb_temp = signal_cb;
    signal_cb = (struct signal_callback_list *)signal_cb->hh.next;

    HASH_DEL(*cb_list, signal_cb_temp);
    free(signal_cb_temp);
  }
}
