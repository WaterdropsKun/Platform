double hist_mean(double *hist,int low,int up)
	//��Ҫ����Ϊ����ֱ��ͼ��Ӧ������ֵ��ֵ��histΪֱ��ͼ��low��up�ֱ�Ϊֱ��ͼ���½���Ͻ�
{
double a=0,b=0;
int i;
for(i=low;i<=up;i++)
{
a+=hist[i]*i;
b+=hist[i];
}
return a/b;
}