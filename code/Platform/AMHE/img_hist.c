   void img_hist(int *img,int m,int n,int binnum,int *hist)
	   //��Ҫ����Ϊ����ͼ������ֱ��ͼ��imgΪͼ��m��n�ֱ�Ϊͼ���������������binNumΪbin��Ŀ��histΪֱ��ͼ
   {
   int i,j;
   for(i=0;i<n;i++)           
   for(j=0;j<m;j++)
    hist[img[i*m+j]]+=1;
   }
   