double min(double *source,int low,int up)
	//��Ҫ����Ϊ����num��Ŀ�����ݵ�source��ȥ��Сֵ
{
int i;
double fmin=source[low];
for(i=low;i<=up;i++)
{
if(source[i]<fmin)
fmin=source[i];
}
return fmin;
}