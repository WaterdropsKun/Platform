#include <math.h>
extern double max(int *source,int low,int up);                                            //���������ֵ����Сֵ
extern double min(int *source,int low,int up);
extern double hist_mean(double *hist,int low,int up);                              

//_inline double *sring_cut(double *string,int begin,int end)                               //����ȡ
//{
	//return &string[begin];
//}

void modified_hist(double *hist,int low,int up,double c,double *img_hist_AM)
	//����Ϊֱ��ͼ������histΪ����ֱ��ͼ��low��upΪֱ��ͼ��Ӧ���½���Ͻ죬cΪ�ԱȶȲ�����img_hist_AMΪ������ֱ��ͼ
{
int num=up-low+1;
double p_min=min(hist,low,up);                                                          
double p_max=max(hist,low,up);
double p_mid=(p_min+p_max)/2;
double xm=hist_mean(hist,low,up);                      //ֱ��ͼ��Ӧ�ľ�ֵ
int Xm=(int)floor(xm);
double xml=hist_mean(hist,low,Xm);                     //ֱ��ͼ�����ľ�ֵ
double xmu=hist_mean(hist,Xm+1,up);                    //ֱ��ͼ�����ľ�ֵ
double alph[256]={0};                                                
int i;
for (i=low;i<=up;i++)
{
if(i<=Xm)
alph[i]=(Xm-xml)/(xmu-xml);
else  alph[i]=(xmu-Xm)/(xmu-xml);
alph[i]*=c;
}
for (i=low;i<=up;i++)
{
if(hist[i]>p_mid)
img_hist_AM[i-low]=p_mid+alph[i]*(hist[i]-p_mid)*(hist[i]-p_mid)/(p_max-p_mid);       //ֱ��ͼ����           
else img_hist_AM[i-low]=p_mid-alph[i]*(p_mid-hist[i])*(p_mid-hist[i])/(p_mid-p_min);  
}
}

