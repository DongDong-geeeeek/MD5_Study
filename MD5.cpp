#include <stdio.h>
#include <math.h>

//表示一组有64个字节数据
#define NUMINGOURP 64

//计算常数-------------------------------------------------------------
//4个md5缓冲器常数
unsigned char md5[16] = { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
						  0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10 };
unsigned int* pMd5 = (unsigned int*)md5;//pMd5[0]、pMd5[1]、pMd5[2]、pMd5[3]，最终结果
//64元素整数常数组
unsigned int CS_64[64];
void Init64(void) {
	//算法：用4294967296乘abs（sin（i））后的值的整数部分
	for (int i = 1; i <= 64; i++) {
		CS_64[i - 1] = (unsigned int)(4294967296 * fabs(sin(i)));
	}
}
//16转换例程常数 7,12,17,22,5,9,14,20,4,11,16,23,6,10,15,21
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
//4个md5缓冲器计算方式或
//unsigned int md5_1 = 0x67452301;
//unsigned int md5_2 = 0xEFCDAB89;
//unsigned int md5_3 = 0x98BADCFE;
//unsigned int md5_4 = 0x10325476;

//计算常数-------------------------------------------------------------

//计算算法-------------------------------------------------------------
unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z);
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z);
//定义函数指针 pFun 用来指向 函数Md5_1、Md5_2....
typedef unsigned int (*pFun)(unsigned int x, unsigned int y, unsigned int z);
unsigned int NewMd5(unsigned int md5_1, unsigned int md5_2, unsigned int md5_3, unsigned int md5_4, pFun fun, unsigned int data, unsigned int cs64, unsigned int cs16);
void GetMD5(unsigned char data[NUMINGOURP]);
//计算算法-------------------------------------------------------------

//VS环境下，32位和64位的计算机，long都是4字节32位的
//gcc环境下，32位机long是32位；64位机long是64位
long g_lDataNytes = 0;
void GetLen(char* filename);
void GetData(char* filename);
void SaveToFile(char* filename);

// argc：命令行参数个数		默认是以1个，也即本软件exe
// argv：命令行参数是什么		argv[0]是软件本身的路径 argv[1]、argv[2]..是命令行传递给程序的参数
// argd：环境目录的一些配置信息
int main(int argc, char* argv[]/*, char* argd[]*/) {
	Init64();
	GetLen(argv[1]);
	GetData(argv[1]);
	SaveToFile(argv[2]); //第二个命令行参数为文件名
	printf("文件 %s 的md5码已生成。\n存放在文件 %s 中。", argv[1], argv[2]);
	return 0;
}

unsigned int Md5_1(unsigned int x, unsigned int y, unsigned int z) {
	return ((x & y) | ((~x) & z));
}
unsigned int Md5_2(unsigned int x, unsigned int y, unsigned int z) {
	return ((x & z) | (y & (~z)));
}
unsigned int Md5_3(unsigned int x, unsigned int y, unsigned int z) {
	return (x ^ y ^ z);
}
unsigned int Md5_4(unsigned int x, unsigned int y, unsigned int z) {
	return (y ^ (x | (~z)));
}
unsigned int NewMd5(unsigned int md5_1, unsigned int md5_2, unsigned int md5_3, unsigned int md5_4, pFun fun, unsigned int data, unsigned int cs64, unsigned int cs16) {
	unsigned int md5_Temp = md5_1 + fun(md5_2, md5_3, md5_4) + data + cs64;
	md5_Temp = (md5_Temp << cs16) | (md5_Temp >> (32 - cs16));
	md5_1 = md5_2 + md5_Temp;
	return md5_1;
}
void GetLen(char* filename) {
	//打开文件
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "r");
	if (res != 0 || pFile == NULL) {
		return;
	}

	//定位到文件尾部
	fseek(pFile, 0, SEEK_END);
	//获取文件字符数
	g_lDataNytes = ftell(pFile);
	//关闭文件
	fclose(pFile);
}
void GetData(char* filename) {
	int iFullGroupNum = g_lDataNytes / NUMINGOURP;		//满64字节的组数
	int iLastGroupEleNum = g_lDataNytes % NUMINGOURP;	//最后一个不满64字节的组中有多少字节

	//处理数据
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "r");
	if (res != 0 || pFile == NULL) {
		return;
	}

	//处理满组
	for (int i = 0; i < iFullGroupNum; i++) {
		unsigned char buffer[NUMINGOURP] = { 0 };
		fread(buffer, 1, NUMINGOURP, pFile);
		//md5运算
		GetMD5(buffer);
	}
	//处理最后一组-----------------------------------------------------------------------------
	//1.最后一组数据量大于0小于等于56Bytes
	if (iLastGroupEleNum > 0 && iLastGroupEleNum <= 56) {
		//读数据
		unsigned char buffer[NUMINGOURP] = { 0 };
		int count = fread(buffer, 1, NUMINGOURP, pFile);	//count表示本次读取了多少Bytes数据

		//最后8字节赋值位数
		*(unsigned long long*)(buffer + 56) = (unsigned long long)g_lDataNytes * 8;

		//中间空挡补值100000..
		if (count < 56) {
			buffer[count] = /*(unsigned char)*/0x80;
			for (size_t i = count + 1; i < 56; i++) {
				buffer[i] = 0;
			}
		}
		//md5运算
		GetMD5(buffer);
	}
	//2.最后一组数据量等于64Bytes
	else if (0 == iLastGroupEleNum) {
		//新空间
		unsigned char buffer[NUMINGOURP] = { 0 };
		//最后8字节填写位数
		*(unsigned long	long*)(buffer + 56) = (unsigned long long)g_lDataNytes * 8;
		//补值100000....
		buffer[0] = 0x80;
		for (int i = 1; i < 56; i++) {
			buffer[i] = 0;
		}
		//// 打印数据以验证是否正确
		//for (int i = 0; i < 56; i++) {
		//	printf("%c  %x \n", buffer[i], buffer[i]);
		//}
		//printf("%lld", *(unsigned long long*)(buffer + 56));

		//md5运算
		GetMD5(buffer);
	}
	//3.最后一组数据量大于56Bytes但是小于64Bytes
	else if (iLastGroupEleNum > 56 && iLastGroupEleNum < 64) {
		//第一部分 读出最后一组
		unsigned char buffer[NUMINGOURP] = { 0 };
		int count = fread(buffer, 1, NUMINGOURP, pFile);
		// 补充值10000....
		buffer[count] = 0x80;
		for (int i = count + 1; i < NUMINGOURP; i++) {
			buffer[i] = 0;
		}
		//md5运算
		GetMD5(buffer);
		//第二部分 申请新空间
		unsigned char newBuffer[NUMINGOURP] = { 0 };
		*(unsigned long long*)(newBuffer + 56) = (unsigned long long)g_lDataNytes * 8;
		for (int i = 0; i < 56; i++) {
			newBuffer[i] = 0;
		}
		//md5运算
		GetMD5(newBuffer);
	}
	//处理最后一组-----------------------------------------------------------------------------

	fclose(pFile);
}
void GetMD5(unsigned char data[NUMINGOURP]) {
	unsigned int* pData = (unsigned int*)data;	//将一组64字节的数转为16个4字节的unsinged int
	unsigned int A = pMd5[0];
	unsigned int B = pMd5[1];
	unsigned int C = pMd5[2];
	unsigned int D = pMd5[3];
	//第一轮
	//[ABCD 0(data) 7(CS16) 1(CS64)]		[DABC 1 12 2]		[CDAB 2 17 3]		[BCDA 3 22 4] 
	//[ABCD 4 7 5]							[DABC 5 12 6]		[CDAB 6 17 7]		[BCDA 7 22 8]
	//[ABCD 8 7 9]							[DABC 9 12 10]		[CDAB 10 17 11]		[BCDA 11 22 12]
	//[ABCD 12 7 13]						[DABC 13 12 14]		[CDAB 14 17 15]		[BCDA 15 22 16]
	A = NewMd5(A, B, C, D, Md5_1, pData[0], CS_64[0], CS11);
	D = NewMd5(D, A, B, C, Md5_1, pData[1], CS_64[1], CS12);
	C = NewMd5(C, D, A, B, Md5_1, pData[2], CS_64[2], CS13);
	B = NewMd5(B, C, D, A, Md5_1, pData[3], CS_64[3], CS14);

	A = NewMd5(A, B, C, D, Md5_1, pData[4], CS_64[4], CS11);
	D = NewMd5(D, A, B, C, Md5_1, pData[5], CS_64[5], CS12);
	C = NewMd5(C, D, A, B, Md5_1, pData[6], CS_64[6], CS13);
	B = NewMd5(B, C, D, A, Md5_1, pData[7], CS_64[7], CS14);

	A = NewMd5(A, B, C, D, Md5_1, pData[8], CS_64[8], CS11);
	D = NewMd5(D, A, B, C, Md5_1, pData[9], CS_64[9], CS12);
	C = NewMd5(C, D, A, B, Md5_1, pData[10], CS_64[10], CS13);
	B = NewMd5(B, C, D, A, Md5_1, pData[11], CS_64[11], CS14);

	A = NewMd5(A, B, C, D, Md5_1, pData[12], CS_64[12], CS11);
	D = NewMd5(D, A, B, C, Md5_1, pData[13], CS_64[13], CS12);
	C = NewMd5(C, D, A, B, Md5_1, pData[14], CS_64[14], CS13);
	B = NewMd5(B, C, D, A, Md5_1, pData[15], CS_64[15], CS14);

	//第二轮
	//[ABCD 1 5 17]		[DABC 6 9 18]	[CDAB 11 14 19]		[BCDA 0 20 20]
	//[ABCD 5 5 21]		[DABC 10 9 22]	[CDAB 15 14 23]		[BCDA 4 20 24]
	//[ABCD 9 5 25]		[DABC 14 9 26]	[CDAB 3 14 27]		[BCDA 8 20 28]
	//[ABCD 13 5 29]	[DABC 2 9 30]	[CDAB 7 14 31]		[BCDA 12 20 32]
	A = NewMd5(A, B, C, D, Md5_2, pData[1], CS_64[16], CS21);
	D = NewMd5(D, A, B, C, Md5_2, pData[6], CS_64[17], CS22);
	C = NewMd5(C, D, A, B, Md5_2, pData[11], CS_64[18], CS23);
	B = NewMd5(B, C, D, A, Md5_2, pData[0], CS_64[19], CS24);

	A = NewMd5(A, B, C, D, Md5_2, pData[5], CS_64[20], CS21);
	D = NewMd5(D, A, B, C, Md5_2, pData[10], CS_64[21], CS22);
	C = NewMd5(C, D, A, B, Md5_2, pData[15], CS_64[22], CS23);
	B = NewMd5(B, C, D, A, Md5_2, pData[4], CS_64[23], CS24);

	A = NewMd5(A, B, C, D, Md5_2, pData[9], CS_64[24], CS21);
	D = NewMd5(D, A, B, C, Md5_2, pData[14], CS_64[25], CS22);
	C = NewMd5(C, D, A, B, Md5_2, pData[3], CS_64[26], CS23);
	B = NewMd5(B, C, D, A, Md5_2, pData[8], CS_64[27], CS24);

	A = NewMd5(A, B, C, D, Md5_2, pData[13], CS_64[28], CS21);
	D = NewMd5(D, A, B, C, Md5_2, pData[2], CS_64[29], CS22);
	C = NewMd5(C, D, A, B, Md5_2, pData[7], CS_64[30], CS23);
	B = NewMd5(B, C, D, A, Md5_2, pData[12], CS_64[31], CS24);

	//第三轮
	//[ABCD 5 4 33] [DABC 8 11 34] [CDAB 11 16 35] [BCDA 14 23 36]
	//[ABCD 1 4 37] [DABC 4 11 38] [CDAB 7 16 39] [BCDA 10 23 40]
	//[ABCD 13 4 41] [DABC 0 11 42] [CDAB 3 16 43] [BCDA 6 23 44]
	//[ABCD 9 4 45] [DABC 12 11 46] [CDAB 15 16 47] [BCDA 2 23 48]
	A = NewMd5(A, B, C, D, Md5_3, pData[5], CS_64[32], CS31);
	D = NewMd5(D, A, B, C, Md5_3, pData[8], CS_64[33], CS32);
	C = NewMd5(C, D, A, B, Md5_3, pData[11], CS_64[34], CS33);
	B = NewMd5(B, C, D, A, Md5_3, pData[14], CS_64[35], CS34);

	A = NewMd5(A, B, C, D, Md5_3, pData[1], CS_64[36], CS31);
	D = NewMd5(D, A, B, C, Md5_3, pData[4], CS_64[37], CS32);
	C = NewMd5(C, D, A, B, Md5_3, pData[7], CS_64[38], CS33);
	B = NewMd5(B, C, D, A, Md5_3, pData[10], CS_64[39], CS34);

	A = NewMd5(A, B, C, D, Md5_3, pData[13], CS_64[40], CS31);
	D = NewMd5(D, A, B, C, Md5_3, pData[0], CS_64[41], CS32);
	C = NewMd5(C, D, A, B, Md5_3, pData[3], CS_64[42], CS33);
	B = NewMd5(B, C, D, A, Md5_3, pData[6], CS_64[43], CS34);

	A = NewMd5(A, B, C, D, Md5_3, pData[9], CS_64[44], CS31);
	D = NewMd5(D, A, B, C, Md5_3, pData[12], CS_64[45], CS32);
	C = NewMd5(C, D, A, B, Md5_3, pData[15], CS_64[46], CS33);
	B = NewMd5(B, C, D, A, Md5_3, pData[2], CS_64[47], CS34);

	//第四轮
	//[ABCD 0 6 49][DABC 7 10 50][CDAB 14 15 51][BCDA 5 21 52]
	//[ABCD 12 6 53][DABC 3 10 54][CDAB 10 15 55][BCDA 1 21 56]
	//[ABCD 8 6 57][DABC 15 10 58][CDAB 6 15 59][BCDA 13 21 60]
	//[ABCD 4 6 61][DABC 11 10 62][CDAB 2 15 63][BCDA 9 21 64]
	A = NewMd5(A, B, C, D, Md5_4, pData[0], CS_64[48], CS41);
	D = NewMd5(D, A, B, C, Md5_4, pData[7], CS_64[49], CS42);
	C = NewMd5(C, D, A, B, Md5_4, pData[14], CS_64[50], CS43);
	B = NewMd5(B, C, D, A, Md5_4, pData[5], CS_64[51], CS44);

	A = NewMd5(A, B, C, D, Md5_4, pData[12], CS_64[52], CS41);
	D = NewMd5(D, A, B, C, Md5_4, pData[3], CS_64[53], CS42);
	C = NewMd5(C, D, A, B, Md5_4, pData[10], CS_64[54], CS43);
	B = NewMd5(B, C, D, A, Md5_4, pData[1], CS_64[55], CS44);

	A = NewMd5(A, B, C, D, Md5_4, pData[8], CS_64[56], CS41);
	D = NewMd5(D, A, B, C, Md5_4, pData[15], CS_64[57], CS42);
	C = NewMd5(C, D, A, B, Md5_4, pData[6], CS_64[58], CS43);
	B = NewMd5(B, C, D, A, Md5_4, pData[13], CS_64[59], CS44);

	A = NewMd5(A, B, C, D, Md5_4, pData[4], CS_64[60], CS41);
	D = NewMd5(D, A, B, C, Md5_4, pData[11], CS_64[61], CS42);
	C = NewMd5(C, D, A, B, Md5_4, pData[2], CS_64[62], CS43);
	B = NewMd5(B, C, D, A, Md5_4, pData[9], CS_64[63], CS44);
	pMd5[0] += A;
	pMd5[1] += B;
	pMd5[2] += C;
	pMd5[3] += D;
}
void SaveToFile(char* filename) {
	FILE* pFile = NULL;
	errno_t res = fopen_s(&pFile, filename, "a");
	if (res != 0 || pFile == NULL) {
		return;
	}
	fclose(pFile);

	res = fopen_s(&pFile, filename, "w");
	if (res != 0 || pFile == NULL) {
		return;
	}
	for (int i = 0; i < 16; i++) {
		fprintf(pFile, "%x", md5[i]);
	}

	fclose(pFile);
}
