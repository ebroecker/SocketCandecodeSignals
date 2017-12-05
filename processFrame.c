#include <stdio.h>
#include "processFrame.h"
#include "stdbool.h"
#include "libcan-encode-decode/include/can_encode_decode_inl.h"

float toPhysicalValue(uint64_t target, float factor, float offset,
bool is_signed);
uint64_t extractSignal(const uint8_t* frame, const uint8_t startbit,
		const uint8_t length, bool is_big_endian, bool is_signed);

void add_callback(struct signal_callback_list **callbackList,
		struct frame_struct *frame, struct signal_struct *signal,
		void (*callback)(char *, __u64, double, struct timeval, char *device),
		__u8 onChange)
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

void processFrame(struct signal_callback_list *callbackList,
		struct can_frame *cf, struct timeval tv, char *device)
{
	struct signal_callback_list *callbackItem;

	for (callbackItem = callbackList; callbackItem != NULL; callbackItem =
			callbackItem->hh.next)
	{

		if (callbackItem->frame->canID == cf->can_id)
		{
			__u64 value = 0;
			double scaled = 0.;
			unsigned int muxerVal = 0;
			if (callbackItem->signal == NULL)
			{
				struct signal_struct *signal;

				if (callbackItem->frame->isMultiplexed)
				{
					// find multiplexer:
					for (signal = callbackItem->frame->signals; signal != NULL;
							signal = signal->hh.next)
					{
						if (signal->isMultiplexer == 1)
						{
							muxerVal = extractSignal(cf->data, signal->startBit,
									signal->signalLength,
									(bool) signal->is_big_endian,
									signal->is_signed);
							value = extractSignal(cf->data, signal->startBit,
									signal->signalLength,
									(bool) signal->is_big_endian,
									signal->is_signed);
							scaled = toPhysicalValue(value, signal->factor,
									signal->offset, signal->is_signed);
							(callbackItem->callback)(signal->name, value,
									scaled, tv, device);
							break;
						}
					}
					for (signal = callbackItem->frame->signals; signal != NULL;
							signal = signal->hh.next)
					{
						// decode not multiplexed signals and signals with correct muxVal
						if (signal->isMultiplexer == 0
								|| (signal->isMultiplexer == 2
										&& signal->muxId == muxerVal))
						{
							value = extractSignal(cf->data, signal->startBit,
									signal->signalLength,
									(bool) signal->is_big_endian,
									signal->is_signed);
							scaled = toPhysicalValue(value, signal->factor,
									signal->offset, signal->is_signed);
							(callbackItem->callback)(signal->name, value,
									scaled, tv, device);
						}
					}

				}
				else
					for (signal = callbackItem->frame->signals; signal != NULL;
							signal = signal->hh.next)
					{
						value = extractSignal(cf->data, signal->startBit,
								signal->signalLength,
								(bool) signal->is_big_endian,
								signal->is_signed);
						scaled = toPhysicalValue(value, signal->factor,
								signal->offset, signal->is_signed);
						(callbackItem->callback)(signal->name, value, scaled,
								tv, device);
					}

			}
			else
			{
				if ((callbackItem->onChange == 0)
						|| (callbackItem->rawValue != value))
				{
					callbackItem->rawValue = value;
					(callbackItem->callback)(callbackItem->signal->name, value,
							scaled, tv, device);
				}
			}
		}
	}
}
