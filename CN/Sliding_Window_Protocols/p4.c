/* Protocol 4 (sliding window) is bidirectional 
   and is more robust than protocol 3. 
 */

#define MAX_SEQ 1          /* must be 1 for protocol 4 */

typedef enum {frame_arrival, cksum_err, timeout} event_type;

#include "protocol.h"

void
protocol4(void)
{
  seq_nr next_frame_to_send; /* 0 or 1 only */
  seq_nr frame_expected;     /* 0 or 1 only */
  packet buffer;             /* current packet being sent */
  frame r, s;                /* scratch frame variables */
  event_type event;

  /* initializations */
  next_frame_to_send = 0;  /* next frame on the outbound stream */
  frame_expected = 0;      /* number of frame arriving frame expected */
  from_network_layer(&buffer); /* fetch a packet from the network layer */
  s.info = buffer;         /* prepare to send the initial frame */
  s.seq = next_frame_to_send;  /* insert sequence number into frame */
  s.ack = 1 - frame_expected;  /* piggybacked ack */
  to_physical_layer(&s);   /* transmit the frame */
  start_timer(s.seq);      /* start the timer running */

  while (true)
  {
    wait_for_event(&event); /* could be: frame_arrival, cksum_err, timeout */
    if (event == frame_arrival) /* a frame has arrived undamaged. */
    {                      
      from_physical_layer(&r); /* go get it */

      if (r.seq == frame_expected)
      {
        /* Handle inbound frame stream. */
        to_network_layer(&r.info); /* pass packet to network layer */
        inc(frame_expected); /* invert sequence number expected next */
      }

      if (r.ack == next_frame_to_send) /* handle outbound frame stream. */
      {                    
        from_network_layer(&buffer); /* fetch new packet from network layer */
        inc(next_frame_to_send); /* invert sender's sequence number */
      }
    }

    s.info = buffer;            /* construct outbound frame */
    s.seq = next_frame_to_send; /* insert sequence number into it */
    s.ack = 1 - frame_expected; /* seq number of last received frame */
    to_physical_layer(&s);      /* transmit a frame */
    start_timer(s.seq);         /* start the timer running */
  }
}