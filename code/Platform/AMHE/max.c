double max(double *source,int low,int up)
	//��Ҫ����Ϊ����num��Ŀ�����ݵ�source��ȥ��Сֵ
{
int i;
double fmax=source[low];
for(i=low;i<=up;i++)
{
if(source[i]>fmax)
fmax=source[i];
}
return fmax;
}