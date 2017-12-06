#include <stdio.h>
#include "processFrame.h"
#include "stdbool.h"
#include "libcan-encode-decode/include/can_encode_decode_inl.h"

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

	for (callbackItem = callbackList; callbackItem != NULL; callbackItem = callbackItem->hh.next)
	{
		if (callbackItem->frame->canID == cf->can_id)
		{
			if (callbackItem->frame->isMultiplexed)
			{
				// find multiplexer:
				for (signal = callbackItem->frame->signals; signal != NULL; signal = signal->hh.next)
				{
					if (1 == signal->isMultiplexer)
					{
						muxerVal = extractSignal(cf->data, signal->startBit, signal->signalLength, (bool) signal->is_big_endian, signal->is_signed);
						scaled = toPhysicalValue(muxerVal, signal->factor, signal->offset, signal->is_signed);
						(callbackItem->callback)(signal->name, muxerVal, scaled, tv, device);
						break;
					}
				}
			}

			if (callbackItem->signal == NULL)
			{
				if (callbackItem->frame->isMultiplexed)
				{
					for (signal = callbackItem->frame->signals; signal != NULL; signal = signal->hh.next)
					{
						// decode not multiplexed signals and signals with correct muxVal
						if (0 == signal->isMultiplexer || (2 == signal->isMultiplexer && signal->muxId == muxerVal))
						{
							value = extractSignal(cf->data, signal->startBit, signal->signalLength, (bool) signal->is_big_endian, signal->is_signed);
							scaled = toPhysicalValue(value, signal->factor, signal->offset, signal->is_signed);
							(callbackItem->callback)(signal->name, value, scaled, tv, device);
						}
					}

				}
				else
				{
					for (signal = callbackItem->frame->signals; signal != NULL; signal = signal->hh.next)
					{
						value = extractSignal(cf->data, signal->startBit, signal->signalLength, (bool) signal->is_big_endian, signal->is_signed);
						scaled = toPhysicalValue(value, signal->factor, signal->offset, signal->is_signed);
						(callbackItem->callback)(signal->name, value, scaled, tv, device);
					}
				}
			}
			else
			{
				if (callbackItem->frame->isMultiplexed)
				{
					if (0 == callbackItem->signal->isMultiplexer || (2 == callbackItem->signal->isMultiplexer && callbackItem->signal->muxId == muxerVal))
					{
						value = extractSignal(cf->data, callbackItem->signal->startBit, callbackItem->signal->signalLength, (bool) callbackItem->signal->is_big_endian,
								callbackItem->signal->is_signed);
						scaled = toPhysicalValue(value, callbackItem->signal->factor, callbackItem->signal->offset, callbackItem->signal->is_signed);
						callbackItem->rawValue = value;
						(callbackItem->callback)(callbackItem->signal->name, value, scaled, tv, device);
					}
				}
				else
				{
					if ((0 == callbackItem->onChange) || (callbackItem->rawValue != value))
					{
						value = extractSignal(cf->data, callbackItem->signal->startBit, callbackItem->signal->signalLength, (bool) callbackItem->signal->is_big_endian,
								callbackItem->signal->is_signed);
						scaled = toPhysicalValue(value, callbackItem->signal->factor, callbackItem->signal->offset, callbackItem->signal->is_signed);
						callbackItem->rawValue = value;
						(callbackItem->callback)(callbackItem->signal->name, value, scaled, tv, device);
					}
				}
			}
		}
	}
}


