#include <stdio.h>
#include <math.h>

//数据分组
#define ZU  64
long g_lDataBytes = 0;
void GetLen(char* filename);
void GetData(char* filename);

//计算常数
unsigned char md5[16] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
unsigned int* pMd5 = (unsigned int*)md5; //pMd5[0] pMd5[1] pMd5[2] pMd5[3]
unsigned int CS_64[64];
void Init64(void);
//7,12,17,22, 5,9,14,20, 4,11,16,23, 6,10,15,21
#define CS11 7
#define CS12 12
#define CS13 17
#define CS14 22
#define CS21 5
#define CS22 9
#define CS23 14
#define CS24 20
#define CS31 4
#define CS32 11
#define CS33 16
#define CS34 23
#define CS41 6
#define CS42 10
#define CS43 15
#define CS44 21

//计算算法
typedef unsigned int (*pFun)(unsigned int, unsigned int, unsigned int);
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z);
unsigned int NewMD5(unsigned int md1, unsigned int md2,unsigned int md3, unsigned int md4,pFun fun, unsigned int data, unsigned int cs64, unsigned int cs16);
void GetMD5(char data[ZU]);
void Save(char* filename);

int main(int argc, char* argv[])
{
	Init64();
	GetLen(argv[1]);
	GetData(argv[1]);
	Save(argv[2]);
	
	return 0;
}
void Save(char* filename)
{
	for (int i = 0; i < 16; i++)
	{
		if (md5[i] <= 0x0f)
			printf("0%x", md5[i]);
		else
			printf("%x", md5[i]);
	}
		
	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "w");
	if (0 != res || NULL == pFile)
		return;

	for (int i = 0; i < 16; i++)
	{
		if (md5[i] <= 0x0f)
		    fprintf(pFile, "0%x", md5[i]);
		else
		    fprintf(pFile, "%x", md5[i]);
	}
	    

	//关闭文件
	fclose(pFile);
}
void GetMD5(char data[ZU])
{
	unsigned int* pData = (unsigned int*)data;
	unsigned int A = pMd5[0];
	unsigned int B = pMd5[1];
	unsigned int C = pMd5[2];
	unsigned int D = pMd5[3];
	//[ABCD 0 7 1] [DABC 1 12 2] [CDAB 2 17 3] [BCDA 3 22 4]
	//[ABCD 4 7 5] [DABC 5 12 6] [CDAB 6 17 7] [BCDA 7 22 8]
	//[ABCD 8 7 9] [DABC 9 12 10] [CDAB 10 17 11] [BCDA 11 22 12]
	//[ABCD 12 7 13] [DABC 13 12 14] [CDAB 14 17 15] [BCDA 15 22 16]
	A = NewMD5(A, B, C, D, Md5_1, pData[0], CS_64[0], CS11);
	D = NewMD5(D, A, B, C, Md5_1, pData[1], CS_64[1], CS12);
	C = NewMD5(C, D, A, B, Md5_1, pData[2], CS_64[2], CS13);
	B = NewMD5(B, C, D, A, Md5_1, pData[3], CS_64[3], CS14);
	A = NewMD5(A, B, C, D, Md5_1, pData[4], CS_64[4], CS11);
	D = NewMD5(D, A, B, C, Md5_1, pData[5], CS_64[5], CS12);
	C = NewMD5(C, D, A, B, Md5_1, pData[6], CS_64[6], CS13);
	B = NewMD5(B, C, D, A, Md5_1, pData[7], CS_64[7], CS14);
	A = NewMD5(A, B, C, D, Md5_1, pData[8], CS_64[8], CS11);
	D = NewMD5(D, A, B, C, Md5_1, pData[9], CS_64[9], CS12);
	C = NewMD5(C, D, A, B, Md5_1, pData[10], CS_64[10], CS13);
	B = NewMD5(B, C, D, A, Md5_1, pData[11], CS_64[11], CS14);
	A = NewMD5(A, B, C, D, Md5_1, pData[12], CS_64[12], CS11);
	D = NewMD5(D, A, B, C, Md5_1, pData[13], CS_64[13], CS12);
	C = NewMD5(C, D, A, B, Md5_1, pData[14], CS_64[14], CS13);
	B = NewMD5(B, C, D, A, Md5_1, pData[15], CS_64[15], CS14);

	//[ABCD 1 5 17] [DABC 6 9 18] [CDAB 11 14 19] [BCDA 0 20 20]
	//[ABCD 5 5 21] [DABC 10 9 22] [CDAB 15 14 23] [BCDA 4 20 24]
	//[ABCD 9 5 25] [DABC 14 9 26] [CDAB 3 14 27] [BCDA 8 20 28]
	//[ABCD 13 5 29] [DABC 2 9 30] [CDAB 7 14 31] [BCDA 12 20 32]
	A = NewMD5(A, B, C, D, Md5_2, pData[1], CS_64[16], CS21);
	D = NewMD5(D, A, B, C, Md5_2, pData[6], CS_64[17], CS22);
	C = NewMD5(C, D, A, B, Md5_2, pData[11], CS_64[18], CS23);
	B = NewMD5(B, C, D, A, Md5_2, pData[0], CS_64[19], CS24);
	A = NewMD5(A, B, C, D, Md5_2, pData[5], CS_64[20], CS21);
	D = NewMD5(D, A, B, C, Md5_2, pData[10], CS_64[21], CS22);
	C = NewMD5(C, D, A, B, Md5_2, pData[15], CS_64[22], CS23);
	B = NewMD5(B, C, D, A, Md5_2, pData[4], CS_64[23], CS24);
	A = NewMD5(A, B, C, D, Md5_2, pData[9], CS_64[24], CS21);
	D = NewMD5(D, A, B, C, Md5_2, pData[14], CS_64[25], CS22);
	C = NewMD5(C, D, A, B, Md5_2, pData[3], CS_64[26], CS23);
	B = NewMD5(B, C, D, A, Md5_2, pData[8], CS_64[27], CS24);
	A = NewMD5(A, B, C, D, Md5_2, pData[13], CS_64[28], CS21);
	D = NewMD5(D, A, B, C, Md5_2, pData[2], CS_64[29], CS22);
	C = NewMD5(C, D, A, B, Md5_2, pData[7], CS_64[30], CS23);
	B = NewMD5(B, C, D, A, Md5_2, pData[12], CS_64[31], CS24);
	//[ABCD 5 4 33] [DABC 8 11 34] [CDAB 11 16 35] [BCDA 14 23 36]
	//[ABCD 1 4 37] [DABC 4 11 38] [CDAB 7 16 39] [BCDA 10 23 40]
	//[ABCD 13 4 41] [DABC 0 11 42] [CDAB 3 16 43] [BCDA 6 23 44]
	//[ABCD 9 4 45] [DABC 12 11 46] [CDAB 15 16 47] [BCDA 2 23 48]
	A = NewMD5(A, B, C, D, Md5_3, pData[5], CS_64[32], CS31);
	D = NewMD5(D, A, B, C, Md5_3, pData[8], CS_64[33], CS32);
	C = NewMD5(C, D, A, B, Md5_3, pData[11], CS_64[34], CS33);
	B = NewMD5(B, C, D, A, Md5_3, pData[14], CS_64[35], CS34);
	A = NewMD5(A, B, C, D, Md5_3, pData[1], CS_64[36], CS31);
	D = NewMD5(D, A, B, C, Md5_3, pData[4], CS_64[37], CS32);
	C = NewMD5(C, D, A, B, Md5_3, pData[7], CS_64[38], CS33);
	B = NewMD5(B, C, D, A, Md5_3, pData[10], CS_64[39], CS34);
	A = NewMD5(A, B, C, D, Md5_3, pData[13], CS_64[40], CS31);
	D = NewMD5(D, A, B, C, Md5_3, pData[0], CS_64[41], CS32);
	C = NewMD5(C, D, A, B, Md5_3, pData[3], CS_64[42], CS33);
	B = NewMD5(B, C, D, A, Md5_3, pData[6], CS_64[43], CS34);
	A = NewMD5(A, B, C, D, Md5_3, pData[9], CS_64[44], CS31);
	D = NewMD5(D, A, B, C, Md5_3, pData[12], CS_64[45], CS32);
	C = NewMD5(C, D, A, B, Md5_3, pData[15], CS_64[46], CS33);
	B = NewMD5(B, C, D, A, Md5_3, pData[2], CS_64[47], CS34);

	//[ABCD 0 6 49] [DABC 7 10 50] [CDAB 14 15 51] [BCDA 5 21 52]
	//[ABCD 12 6 53] [DABC 3 10 54] [CDAB 10 15 55] [BCDA 1 21 56]
	//[ABCD 8 6 57] [DABC 15 10 58] [CDAB 6 15 59] [BCDA 13 21 60]
	//[ABCD 4 6 61] [DABC 11 10 62] [CDAB 2 15 63] [BCDA 9 21 64]
	A = NewMD5(A, B, C, D, Md5_4, pData[0], CS_64[48], CS41);
	D = NewMD5(D, A, B, C, Md5_4, pData[7], CS_64[49], CS42);
	C = NewMD5(C, D, A, B, Md5_4, pData[14], CS_64[50], CS43);
	B = NewMD5(B, C, D, A, Md5_4, pData[5], CS_64[51], CS44);
	A = NewMD5(A, B, C, D, Md5_4, pData[12], CS_64[52], CS41);
	D = NewMD5(D, A, B, C, Md5_4, pData[3], CS_64[53], CS42);
	C = NewMD5(C, D, A, B, Md5_4, pData[10], CS_64[54], CS43);
	B = NewMD5(B, C, D, A, Md5_4, pData[1], CS_64[55], CS44);
	A = NewMD5(A, B, C, D, Md5_4, pData[8], CS_64[56], CS41);
	D = NewMD5(D, A, B, C, Md5_4, pData[15], CS_64[57], CS42);
	C = NewMD5(C, D, A, B, Md5_4, pData[6], CS_64[58], CS43);
	B = NewMD5(B, C, D, A, Md5_4, pData[13], CS_64[59], CS44);
	A = NewMD5(A, B, C, D, Md5_4, pData[4], CS_64[60], CS41);
	D = NewMD5(D, A, B, C, Md5_4, pData[11], CS_64[61], CS42);
	C = NewMD5(C, D, A, B, Md5_4, pData[2], CS_64[62], CS43);
	B = NewMD5(B, C, D, A, Md5_4, pData[9], CS_64[63], CS44);

	pMd5[0] += A;
	pMd5[1] += B;
	pMd5[2] += C;
	pMd5[3] += D;
}
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & y) | ((~x) & z));
}
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & z) | (y & (~z)));
}
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z)
{
	return  (x ^ y ^ z);
}
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z)
{
	return (y ^ (x | (~z)));
}
unsigned int NewMD5(unsigned int md1, unsigned int md2,unsigned int md3, unsigned int md4,pFun fun, unsigned int data, unsigned int cs64, unsigned int cs16)
{
	unsigned int md5_new = md1 + fun(md2, md3, md4) + data + cs64;
	md5_new = (md5_new << cs16) | (md5_new >> (32 - cs16));
	md1 = md2 + md5_new;
	return md1;
}
void GetLen(char* filename)
{
	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "r");
	if (0 != res || NULL == pFile)
		return;
	//文件指针跳到结尾
	fseek(pFile, 0, SEEK_END);
	//获取位置
	g_lDataBytes = ftell(pFile);
	//关闭文件
	fclose(pFile);
}
void GetData(char* filename)
{
	int man = g_lDataBytes / ZU;
	int fanil = g_lDataBytes % ZU;
	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "r");
	if (0 != res || NULL == pFile)
		return;
	for (int i = 0; i < man; i++)
	{
		char str[ZU] = { 0 };
		fread(str, 1, ZU, pFile);
		//mad5运算
		GetMD5(str);
	}
	//1、 > 0 <= 56
	if (fanil > 0 && fanil <= 56)
	{
		//读数据
		char str[ZU] = { 0 };
		int count = fread(str, 1, ZU, pFile);
		//最后8字节赋值位数
		*(unsigned long long*)(str + 56) = g_lDataBytes * 8;
	    //补充1000
		if (count < 56)
		{
			str[count] = 0x80;
			for (int j = count + 1; j < 56; j++)
			{
				str[j] = 0;
			}
		}
		//mad5运算
		GetMD5(str);
	}
	else if (0 == fanil)
	{
		//新空间
		char str[ZU] = { 0 };
		//最后8字节填位数
		*(unsigned long long*)(str + 56) = g_lDataBytes * 8;
	    //补100000
		str[0] = 0x80;
		for (int j = 1; j < 56; j++)
			str[j] = 0;
		//mad5运算
		GetMD5(str);
	}
	else if (fanil > 56 && fanil < 64)
	{
		//第一部分 读数据
		char str[ZU] = { 0 };
		int count = fread(str, 1, ZU, pFile);
		//补充10000
		str[count] = 0x80;
		for (int j = count + 1; j < 64 ; j++)
		{
			str[j] = 0;
		}
		//md5算法
		GetMD5(str);
		char strNew[ZU] = { 0 };
		*(unsigned long long*)(strNew + 56) = g_lDataBytes * 8;
		//md5算法
		GetMD5(strNew);
	}
    
	//关闭文件
	fclose(pFile);
}
void Init64(void)
{
	//4294967296乘abs(sin(i))后的值的整数部分
	for (int i = 1; i <= 64; i++)
		CS_64[i - 1] = (unsigned int)(4294967296 * fabs(sin(i)));
}
