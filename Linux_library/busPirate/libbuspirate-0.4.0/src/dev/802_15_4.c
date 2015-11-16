#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <802_15_4.h>

void mac_addr_dump(FILE * stream, mac_addr * addr) {
  assert((addr->mode == 0x02) || (addr->mode == 0x03));

  switch (addr->mode) {
  case 0x02:
    fprintf(stream, "panid=%.4X;sadr=%.4X", addr->m2.panid, addr->m2.sadr);
    break;
  case 0x03:
    fprintf(stream, "ladr=%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X",
	    addr->m3.ladr[0], addr->m3.ladr[1], addr->m3.ladr[2],
	    addr->m3.ladr[3], addr->m3.ladr[4], addr->m3.ladr[5],
	    addr->m3.ladr[6], addr->m3.ladr[7]);
    break;
  }
}

void mac_frame_dump(FILE * stream, mac_frame * frame) {
  int i;

  fprintf(stream, " length: %u\n", frame->len);
  fprintf(stream, " seqnum: %u\n", frame->seqnum);
  fprintf(stream, " src   : ");
  mac_addr_dump(stream, &frame->src_addr);
  fprintf(stream, "\n");
  fprintf(stream, " dst   : ");
  mac_addr_dump(stream, &frame->dst_addr);
  fprintf(stream, "\n");
  
  fprintf(stream, " data  :");
  for (i= 0; i < frame->len; i++) {
    fprintf(stream, " %.2X", frame->payload[i]);
    if ((i & 0x0F) == 0x0F)
      fprintf(stream, "\n        ");
  }
  fprintf(stream, " (");
  for (i= 0; i < frame->len; i++) {
    if (isalpha(frame->payload[i]))
      fprintf(stream, " %c", frame->payload[i]);
    else
      fprintf(stream, " .");
    if ((i & 0x0F) == 0x0F)
      fprintf(stream, "\n        ");
  }
  fprintf(stream, " )");  
  fprintf(stream, "\n");

  fprintf(stream, " crc   : %.4X\n", frame->crc);
}
