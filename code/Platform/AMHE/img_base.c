void img_base(int *img,int m,int n,int *basebin,int bin_num,int *baseimg)
	//��Ҫ����Ϊ����bin�����������µ�ͼ��imgΪԭͼ��m��n�ֱ�Ϊͼ���������������basebinΪ��������bin_numΪbin��Ŀ��baseimgΪ��ͼ��
{
int i,j;
for(i=0;i<n;i++)
for(j=0;j<m;j++)
baseimg[i*m+j]=basebin[img[i*m+j]];
}