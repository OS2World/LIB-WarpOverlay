//simple inline implementation for some common functions

void memset (void *output, int value, unsigned int len);
#pragma aux memset=              \
        "rep stosb"              \
        parm [edi] [eax] [ecx]   \
        modify [edi ecx]

void memcpy (void *output, void *input, unsigned int len);
#pragma aux memcpy=              \
        "rep movsb"              \
        parm [edi] [esi] [ecx]   \
        modify [edi esi ecx]

int memcmp (void *s1, void *s2, unsigned int len);
#pragma aux memcmp=              \
        "xor eax,eax"            \
        "repe cmpsb"             \
        "jb @2"                  \
        "seta al"                \
        "jmp @3"                 \
        "@2:"                    \
        "sbb eax,eax"            \
        "@3:"                    \
        parm [esi] [edi] [ecx]   \
        modify [esi edi ecx eax] \
        value [eax]

char * strcpy(char *dst, const char *src);
#pragma aux strcpy=              \
        "xor ecx,ecx"            \
        "dec ecx"                \
        "push edi"               \
        "mov edi,esi"            \
        "xor eax,eax"            \
        "repne scasb"            \
        "neg ecx"                \
        "dec ecx"                \
        "pop edi"                \
        "mov eax,edi"            \
        "rep movsb"              \
        parm [edi] [esi]         \
        value [eax]              \
        modify [edi esi ecx eax]

int  strlen(char *str);
#pragma aux strlen=              \
        "xor ecx,ecx"            \
        "dec ecx"                \
        "xor eax,eax"            \
        "repnz scasb"            \
        "neg ecx"                \
        "dec ecx"                \
        "mov eax,ecx"            \
        parm [edi]               \
        value [eax]              \
        modify [ecx edi eax]

int strcmp (void *s1,void *s2);
#pragma aux strcmp=              \
        "push edi"               \
        "xor ecx,ecx"            \
        "dec ecx"                \
        "push ecx"               \
        "repnz scasb"            \
        "neg ecx"                \
        "mov eax,ecx"            \
        "mov edi,esi"            \
        "pop ecx"                \
        "repnz scasb"            \
        "neg ecx"                \
        "sub eax,ecx"            \
        "jne @1"                 \
        "dec ecx"                \
        "pop edi"                \
        "repe cmpsb"             \
        "je @2"                  \
        "ja @3"                  \
        "mov eax,1"              \
        "jmp @1"                 \
        "@3:"                    \
        "mov eax,-1"             \
        "jmp @1"                 \
        "@2:"                    \
        "xor eax,eax"            \
        "@1:"                    \
        parm [edi] [esi]         \
        value [eax]              \
        modify [ecx edi esi eax]

