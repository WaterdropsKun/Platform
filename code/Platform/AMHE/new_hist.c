void new_hist(double *old_hist,double *newHistOut,int *map,int binNum)
	//��Ҫ����Ϊ��ֱ��ͼӳ����µ�ֱ��ͼ��old_histΪԭֱ��ͼ��newHistOutΪ��ֱ��ͼ��mapΪ����ֵӳ���binNumΪbin��Ŀ
{
int i,j;
for(i=0;i<binNum;i++)                                //��ֱ��ͼ����
newHistOut[i]=0;
for(j=0;j<binNum;j++)
newHistOut[map[j]]+=old_hist[j];   //������ֱ��ͼ
}