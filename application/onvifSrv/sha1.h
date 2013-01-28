#ifndef _SHA1_H_ 
#define _SHA1_H_ 
enum 
{ 
    shaSuccess = 0, 
    shaNull,            /* 空指示参量 */ 
    shaInputTooLong,    /* 输入数据太长提示 */ 
    shaStateError       /* called Input after Result --以输入结果命名之 */ 
}; 
#define SHA1HashSize 20 
 
/* 
 *  以下这种结构将会控制上下文消息 for the SHA-1 
 *  hashing operation 
 */ 
typedef struct SHA1Context 
{ 
    unsigned int Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */ 
 
    unsigned int Length_Low;            /* Message length in bits      */ 
    unsigned int Length_High;           /* Message length in bits      */ 
 
                               /* Index into message block array   */ 
     unsigned   short  Message_Block_Index; 
     unsigned   char Message_Block[64];      /* 512-bit message blocks      */ 
 
    int Computed;               /* Is the digest computed?         */ 
    int Corrupted;             /* Is the message digest corrupted? */ 
} SHA1Context; 
 
/* 
 *  函数原型 
 */ 
int SHA1Reset(  SHA1Context *); 
int SHA1Input(  SHA1Context *, 
                const    unsigned   char *, 
                unsigned int); 
int SHA1Result( SHA1Context *, 
                 unsigned   char  Message_Digest[SHA1HashSize]); 
 


#endif /*_SHA1_H_*/ 


