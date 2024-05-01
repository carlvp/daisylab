#ifndef SyxBulkFormat_H
#define SyxBulkFormat_H

struct Syx {
  using Byte=unsigned char;

  struct Preamble {
    Byte statusByte;        // expected: 0xf0
    Byte manufacturerId;    // expected: 0x43
    Byte substatusCh;       // expected: 0
    Byte format;            // 0x09 for 32-voice bulk format
    Byte byteCount[2];      // big endian 16-bit int, expected 0x20, 0x00
  };

  struct Postamble {
    Byte chksum;
    Byte eox;               // 0xf7
  };
};

struct SyxVoiceParam: public Syx {
  struct Envelope {
    Byte rate[4];           // 0-99
    Byte level[4];          // 0-99
  };
  
  struct KbdLevelScaling {
    Byte bp;                // 0-99
    Byte ld;                // 0-99
    Byte rd;                // 0-99
    Byte rc_lc;             // 0-3 | 0-3 (in bits b3,b2 | b1,b0)
  };

  struct Op {
    struct Envelope envelope;
    struct KbdLevelScaling kls;
    Byte pd_krs;            // 0-14 | 0-7 (in bits b6..b3|b2..b0)
    Byte ts_ams;            // 0-7 | 0-3  (in bits b5..b2|b1,b2)
    Byte tl;                // 0-99
    Byte freqCoarse_pm;     // 0-31 | 0-1 (in bits b5..b1|b0)
    Byte freqFine;          // 0-99
  };

  struct Lfo {
    Byte speed;             // 0-99
    Byte delay;             // 0-99
    Byte pmd;               // 0-99
    Byte amd;               // 0-99
    Byte lpms_wave_sync;    // 0-7|0-5|0-1 (in b6..b4|b3..b1|b0) 
  };
    
  struct Op op[6];
  struct Envelope pitchEnvelope;
  Byte algorithm;         // 0-31
  Byte opi_fbl;           // 0-1 | 0-7 (in b3|b2..b0)
  struct Lfo lfo;
  Byte transpose;         // 0-48
  char name[10];
};

struct SyxBulkFormat: public Syx {
  
  static unsigned constexpr Size=4104;
  
  struct Preamble preamble;
  struct SyxVoiceParam voiceParam[32];
  struct Postamble postamble;

  // Check if a syx buffer has proper preamble
  static bool isSyxBulkFormat(const Byte bytes[Size]) {
    return bytes[0]==0xf0 && bytes[1]==0x43 && bytes[3]==0x09
      && bytes[4]==0x20 && bytes[5]==0x00;
  }
};

#endif
