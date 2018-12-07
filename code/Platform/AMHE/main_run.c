#include <stdlib.h>
extern void makeLUT(int img_Min,int img_Max,int NrBins,int *LUT);
extern void img_base(int *img,int m,int n,int *basebin,int bin_num,int *baseimg);
extern void img_hist(int *img,int m,int n,int binnum,double *hist);
extern void AMHE_run(double *histIN,int binNum, double c, double *newHistOut,int *Map);
void AMHE_main_run(int *img,int m,int n,double c,int binNum,double *newHistOut,int *Map)  
	//imgΪ����ͼ��mΪͼ��������nΪͼ��������binNumΪͼ��bin��Ŀ��newHistOutΪ���ֱ��ͼ��MapΪͼ��ӳ���
{
	int i;
	int *basebin;                                       //ͼ������ת����ָ��bin��������ֵ����
	int *baseimg;                                       //ָ��bin��Ŀת�����ͼ��
	int *hist;                                       //����ͼ��ֱ��ͼ                              
    basebin =(int*)malloc(sizeof(int)*256);   
    makeLUT(0,255,binNum,basebin);                         //����256������ֵ����
    baseimg=(int*)malloc(sizeof(int)*m*n);        
    img_base(img,m,n,basebin,binNum,baseimg);           //����bin��Ŀ��ͼ��
    hist=(int*)malloc(sizeof(int)*binNum);   
	for(i=0;i<binNum;i++)                               //ͼ��ֱ��ͼ��ʼ��Ϊ0
		hist[i]=0;
    img_hist(baseimg,m,n,binNum,hist);                  //����ͼ������ֱ��ͼ
    AMHE_run(hist,binNum,c,newHistOut,Map,m,n);           //�������ֱ��ͼ��ӳ���
    free(basebin);
    free(baseimg);
	free(hist);

}