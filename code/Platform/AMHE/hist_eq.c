#include <stdlib.h>
#include <math.h>
void hist_eq(double *hist_AM,int low,int up,int *eq_index)
	//��Ҫ����Ϊֱ��ͼ���⻯��hist_AM��ֱ��ͼ��low��upΪֱ��ͼ��Ӧ���½���Ͻ磬eq_indexΪ���ɵ�����ֵ����
{
int num=up-low+1;
double *img_culmutive_hist;                                                 //����ֱ��ͼ
int i;
img_culmutive_hist=(double*)malloc(sizeof(double)*num);
img_culmutive_hist[0]=hist_AM[0];
for(i=1;i<num;i++)
 img_culmutive_hist[i] =img_culmutive_hist[i-1]+hist_AM[i];                 //���ɻ���ֱ��ͼ
for(i=0;i<num;i++)
 {
 img_culmutive_hist[i] =img_culmutive_hist[i]/img_culmutive_hist[num-1];    //����ֱ��ͼ��һ��
 eq_index[i]=floor(img_culmutive_hist[i]*(up-low)+low);                     //��������ֵ����
 }
free(img_culmutive_hist);
}