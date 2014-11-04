//a lot of pragmas, to avoid runtime using
void memset (void *output, int value, unsigned long len);
#pragma aux memset=              \
        "rep stosb"              \
        parm [edi] [eax] [ecx]   \
        modify [edi ecx]

void memcpy (void *output, void *input, unsigned long len);
#pragma aux memcpy=              \
        "rep movsb"              \
        parm [edi] [esi] [ecx]   \
        modify [edi esi ebx ecx]

#define FOURCC_Y422 0x32323459
#define FOURCC_LUT8 0x3854554c
#define FOURCC_R565 0x35363552
