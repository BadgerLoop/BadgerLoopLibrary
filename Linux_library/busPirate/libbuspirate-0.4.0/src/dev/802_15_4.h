#ifndef __802_15_4_H__
#define __802_15_4_H__

#include <stdio.h>

typedef struct mac_addr_m2 {
  unsigned short panid;
  unsigned short sadr;
} mac_addr_m2;

#define MAC_ADDR_M3_LEN 8
typedef struct mac_addr_m3 {
  unsigned char ladr[MAC_ADDR_M3_LEN];
} mac_addr_m3;

typedef struct mac_addr {
  unsigned char mode; /* range = 0-3 */
  union {
    mac_addr_m2 m2; /* mode == 0x02 */
    mac_addr_m3 m3; /* mode == 0x03 */
  };
} mac_addr;

#define MAC_FRAME_PAYLOAD_LEN 128
typedef struct mac_frame {
  mac_addr       src_addr;
  mac_addr       dst_addr;
  unsigned char  len;
  unsigned char  type;
  unsigned char  seqnum;
  unsigned char  payload[MAC_FRAME_PAYLOAD_LEN];
  unsigned short crc;

  /*unsigned char security_enabled: 1;
  unsigned char frame_pending   : 1;
  unsigned char ack_request     : 1;
  unsigned char pan_id_compress : 1;
  unsigned char frame_version   : 2;*/

} mac_frame;

#ifdef __cplusplus
extern "C" {
#endif

  void mac_frame_dump(FILE * stream, mac_frame * frame);
  void mac_addr_dump(FILE * stream, mac_addr * addr);

#ifdef __cplusplus
}
#endif

#endif /* __802_15_4_H__ */
