#include <math.h>
#define max(a,b) (a>b)?a:b
#define min(a,b)  (a<=b)?a:b
void makeLUT(int img_Min,int img_Max,int NrBins,int *LUT)
	//��Ҫ����Ϊ����NrBins��Ŀ����ɫ��������img_Min��img_MaxΪ��С����ֵ���������ֵ��NrBinsΪbin����Ŀ��LUTΪ��ɫ������
{
	int i;
	int cc=max(1,img_Min);
    int img_Max1=img_Max+cc-img_Min;
    int img_Min1=cc;
    int BinSize=(int)(floor(1+(double)(img_Max-img_Min)/NrBins));    //ÿ��bin�Ŀ��
    for( i=img_Min1;i<=img_Max1;i++)
    LUT[i-img_Min1]=(int)(floor((double)(i-img_Min1)/BinSize));
}