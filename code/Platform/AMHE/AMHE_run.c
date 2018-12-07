#include <stdlib.h>
extern int OstuCompute(double *hist,int binNum);
extern void modified_hist(double *hist,int low,int up,double c,double *img_hist_AM);
extern void hist_eq(double *hist_AM,int low,int up,int *eq_index);
extern void new_hist(double *old_hist,double *newHistOut,int *map,int binNum);
void AMHE_run(int *histIN,int binNum, double c, double *newHistOut,int *Map,int m,int n)
	//histINΪ����ֱ��ͼ��binNumΪbin��Ŀ��cΪ�Աȶȣ�newHistOutΪ���ֱ��ͼ��MapΪӳ���m��n�ֱ�Ϊͼ�������������
{  
   double *low_hist_AM;                                     //�����������ֱ��ͼ
   double *up_hist_AM;                                      //�����������ֱ��ͼ
   int *low_eq_index;                                       //�������⻯�����ɫ����
   int *up_eq_index;                                        //�������⻯�����ɫ����
   int i;
   int low_num=0;
   int up_num=0;
   int level;
   double *hist;
   hist=(double*)malloc(sizeof(double)*binNum);
   for(i=0;i<binNum;i++)
	   hist[i]=(double)histIN[i]/(m*n);
	level=OstuCompute(hist,binNum);                            //otsu��ֵ��ȡ
   for(i=0;i<=level;i++)                                        //ֱ��ͼ�ֱ��һ��
	   low_num=low_num+histIN[i];
   for(i=0;i<=level;i++)
	   hist[i]=(double)histIN[i]/low_num;
   for(i=level+1;i<binNum;i++)
	   up_num=up_num+histIN[i];
   for(i=level+1;i<binNum;i++)
	    hist[i]=(double)histIN[i]/up_num;
   low_hist_AM=(double*)malloc(sizeof(double)*(level+1));            
   modified_hist(hist,0,level,c,low_hist_AM);               //����ֱ��ͼ���� 
   up_hist_AM=(double*)malloc(sizeof(double)*(binNum-level-1));
   modified_hist(hist,level+1,binNum-1,c,up_hist_AM);       //����ֱ��ͼ����
   low_eq_index=(int*)malloc(sizeof(int)*(level+1));        //�������������⻯����ɫ����
   up_eq_index=(int*)malloc(sizeof(int)*(binNum-level-1)); 
   hist_eq(low_hist_AM,0,level,low_eq_index);               //�԰����������ֱ���⻯
   hist_eq(up_hist_AM,level+1,binNum-1,up_eq_index);
   for( i=0;i<=level;i++)                                  //����ӳ���
	   {
		 if(low_eq_index[i]>=0)
         Map[i]=low_eq_index[i];
		 else Map[i]=0;
        } 
   for(i=0;i<binNum-level-1;i++)
	      {
		  if(up_eq_index[i]<binNum)
          Map[i+level+1]=up_eq_index[i];
		  else Map[i+level+1]=255;
          } 
   new_hist(hist,newHistOut,Map,binNum);                  //�������ֱ��ͼ
   free(hist);
   free(low_eq_index);
   free(up_eq_index);
   free(low_hist_AM);
   free(up_hist_AM);
}