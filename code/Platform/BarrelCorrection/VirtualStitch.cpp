#include <opencv2/core.hpp>

#define VERT_SIZE 3000

float MIN_CUSTOM(float a, float b);
float MAX_CUSTOM(float a, float b);
unsigned char clip(int indata);

//float g_f32HoriRatioComp[VERT_SIZE];
float g_f32VertRatio[VERT_SIZE];
int g_nVertIdxShift[VERT_SIZE];
//float g_f32VertZoomEnd;
unsigned char* pImgTmp = NULL;


float g_f32ScaleRatio;
int outPutFlag = 0; //TODO


/*====================================================================
������      : VirtualStitchInit
����        : ģ���ʼ��
����        :  width: ͼ���[in]
               height: ͼ���[in]
               zoom: ����ϵ��[in]
               endHeight: �������Ե��������߶�[in]
               startHeight:�������Ե�����ʼ�߶�[in]
               pOutWidth: ����Ŵ�ƴ�Ӻ���[out]
               pOutHeight:����Ŵ�ƴ�Ӻ�߶�[out]
			   level���ϲ����Ȳ���
			   upMult:�ϲ�б�ʳ���
			   max_ratio:�²������ı��� (1000-�²�������/1000
			   finalHeight: ���������ͼ��߶�
����ֵ˵��  : �ɹ�: 0, ʧ��: -1
====================================================================*/
int VirtualStitchInit(int width, int height, float zoom, int endHeight, int startHeight, int *pOutWidth, int *pOutHeight, int level, int upMult, float max_ratio, int finalHeight)
{
   int nI=0;
   int nOutH;
   int nFinalWidth;
   int nAdjustEndHeight, nAdjustStartHeight;

   int nMidPoint;

   float f32HoriRatioComp;
   float f32MaxZoomIn, f32Ratio, f32Slope,f32OrgTopWidth;
   float f32VertZoomStart;
   

   float downpart_ratio_d, uppart_ratio_d, ratio_row;
   float ex_ratio;

   FILE* pParaFile = NULL;

	f32MaxZoomIn = zoom;  
    nAdjustEndHeight=endHeight;
    nAdjustStartHeight = startHeight;

    f32OrgTopWidth = width/f32MaxZoomIn;
    *pOutWidth = (int)(width*f32MaxZoomIn);
	for(nI=0;nI<height;nI++)//
    {
		if(nI<nAdjustStartHeight)
			f32Ratio = exp((nAdjustStartHeight-(float)nI)/(level+1000+2*abs(nAdjustStartHeight-nI)));
		else
			f32Ratio = (nAdjustEndHeight-(float)nI)/((float)(nAdjustEndHeight-nAdjustStartHeight));

        f32Ratio = MAX_CUSTOM(f32Ratio, 0);

        nFinalWidth = (f32Ratio*f32OrgTopWidth+(1.0-MIN_CUSTOM(f32Ratio,1.0))*width);
        g_f32VertRatio[nI] = ((float)width)/((float)nFinalWidth);
	}
	for(nI=height-1;nI>=0;nI--)
    {
        if(g_f32VertRatio[nI] <=1.0000001 || nI==height-1)
        {
            f32VertZoomStart = nI;
            g_nVertIdxShift[nI] = nI<<10;
//            g_f32VertZoomEnd = f32VertZoomStart;
        }
        else
        {
            f32VertZoomStart -= g_f32VertRatio[nI];//���ո�ֵ
            g_nVertIdxShift[nI] = (int)(f32VertZoomStart*1024.0);

        }
		if(nI==nAdjustStartHeight)
			nMidPoint = f32VertZoomStart;
    }
	nOutH = height-f32VertZoomStart;//������Ӧ�ĳ����ĸ���
	nMidPoint -= f32VertZoomStart;

	downpart_ratio_d = (1.0 - max_ratio)/(nOutH-nMidPoint-1);//
	uppart_ratio_d = downpart_ratio_d*upMult;
	f32VertZoomStart = nOutH;

	if(finalHeight>0 && finalHeight <nOutH)
		g_f32ScaleRatio = (float)finalHeight / (float)nOutH;
	else
		g_f32ScaleRatio = 1.0;

	for(nI=height-1;nI>=0;nI--)
    {
        if(g_f32VertRatio[nI] <=1.0000001 || nI==height-1)
        {
            f32VertZoomStart -= 1; 
        }
        else
        {
            f32VertZoomStart -= g_f32VertRatio[nI];//���ո�ֵ
        }
		if (f32VertZoomStart>nMidPoint)
			f32HoriRatioComp =  max_ratio + downpart_ratio_d*(f32VertZoomStart-nMidPoint);
		else
			f32HoriRatioComp =f32HoriRatioComp +  uppart_ratio_d*MIN_CUSTOM(exp((f32VertZoomStart-nMidPoint+100.0)/100.0),1.0);//TODO �������100.0������ȡΪ���Ʋ��� 
		
		g_f32VertRatio[nI] *= (f32HoriRatioComp*g_f32ScaleRatio);

	}



	nOutH = nOutH*g_f32ScaleRatio;
	*pOutWidth = (*pOutWidth)*g_f32ScaleRatio;
	*pOutWidth = (*pOutWidth/8)*8;
	if(finalHeight>0)*pOutHeight = finalHeight;
	else *pOutHeight = nOutH;

    pImgTmp = (unsigned char*)malloc((*pOutWidth)*height*3);
    if(pImgTmp == NULL)
    {
        printf("malloc error!");
        return -1;
    }
	errno_t err;
	err = fopen_s(&pParaFile,"Vertical_param_file.txt","w");
//  fprintf(pParaFile,"%d\n",height);
//	fprintf(pParaFile,"%f\n",g_f32VertZoomEnd);
	fprintf(pParaFile,"%f\n",g_f32ScaleRatio);


	for(nI=0;nI<height;nI++)
		fprintf(pParaFile,"%d\n", g_nVertIdxShift[nI]);
	fclose(pParaFile);

    return 0;
}

/*====================================================================
������      : VirtualStitchRelease
����        : ģ��ر�
����ֵ˵��  : �ɹ�: 0, ʧ��: -1
====================================================================*/
int VirtualStitchRelease()
{
    if(pImgTmp != NULL)
        free(pImgTmp);
    return 0;
}



/*====================================================================
������      : VirtualStitchProcess
����        : ����ƴ�Ӵ������庯��
����        :  i_pImg: ����rgbͼ��ָ��[in]
               width: ͼ���[in]
               height: ͼ���[in]
               o_pImg: ���rgbͼ��ָ��[out] 			   
               i_nOutWidth: ����Ŵ�ƴ�Ӻ���[in]
               i_nOutHeight:����Ŵ�ƴ�Ӻ�߶�[in]
����ֵ˵��  : �ɹ�: 0, ʧ��: -1
====================================================================*/
int VirtualStitchProcess(unsigned char * i_pImg, int width, int height, unsigned char * o_pImg, int i_nOutWidth, int i_nOutHeight, CvRect * pRect, int ReCalcFlag, float i_f32Ratio[2], int i_nOrgWidth, int i_nOrgHeight)
{

	float deltx,delty;
	float vary, vary1, varall; 

	FILE * fp=NULL;
    int nI,nJ,nK,nV,nCnt, nCntH;
    int nOrgJ,nOrgJLow,nOrgJHigh;
    int nHalfWidth, nHalfHeight,nFinalWidth,nFinalCenter,nOutFinalCenter, nWidthStep, nOutWidthStep;
	int nOrgHalfWidth, nOrgHalfHeight;
	int nTempCor1,nTempCor2;
	//int nScaleHeight;

    int nCenterX, nCenterY,nAdjustEndHeight, nAdjustStartHeight;
    int nDiff, nWeight1,nWeight2,nWeightSum,nVal;

    unsigned char* pLineL1,* pLineH1, *pOut, *pLineTmp,*pOutTmp;

    float f32Ratio, f32Slope,f32OrgTopWidth;
    int nLeftBound, nRightBound;


    long long s64TmpValue, s64TmpConst0,s64TmpConst1, s64TmpConst2;

    if(i_pImg == NULL || o_pImg == NULL)
    {
        printf("error image pointer!");
        VirtualStitchRelease();
        return -1;
    }

	nOrgHalfWidth = i_nOrgWidth/2;
	nOrgHalfHeight = i_nOrgHeight/2;
    nHalfWidth  = width/2;
    nHalfHeight = height/2;
    nWidthStep = width*3;

	
	nOutWidthStep = i_nOutWidth*3;

	nFinalCenter = width/2;
	nOutFinalCenter = i_nOutWidth/2;

	if(ReCalcFlag==1)
	{
		nLeftBound = 0;
		nRightBound = i_nOutWidth;
	}

	
    for(nI=0;nI<height;nI++)//����Ŵ�
    {
		delty =(nI-nHalfHeight);
		vary = delty*delty;
		


        nFinalWidth = (int)(((float)width)/(g_f32VertRatio[nI]/**g_f32HoriRatioComp[nI]*/));
  
        //nFinalCenter = nCenterX + f32Slope*(nI-nCenterY);
		

        s64TmpConst2 = nFinalCenter<<10;
        s64TmpValue = (long long)(nFinalWidth);
        s64TmpConst0 = (s64TmpValue<<20)/((long long)width);
        s64TmpConst1 = ((s64TmpValue*(long long)nOutFinalCenter)<<10)/((long long)width);

        pLineTmp = (unsigned char*)(i_pImg+nWidthStep*nI);

        nCnt = nI*nOutWidthStep;

		
        for (nJ=0;nJ<i_nOutWidth;nJ++,nCnt+=3)
        {
            nOrgJ = (((long long)nJ*s64TmpConst0)>>10)-s64TmpConst1+s64TmpConst2;

			deltx = (nOrgJ-(nHalfWidth<<10))/1024.0;
			vary1 = deltx*deltx+vary;
			varall = (1.0+ vary1*vary1 * i_f32Ratio[1] +vary1*i_f32Ratio[0]);

            nOrgJLow = nOrgJ>>10;
            nOrgJHigh = nOrgJLow+1;
            nDiff=nOrgJ-(nOrgJLow<<10);
			

            //if(nOrgJHigh>nOrgJLow && nOrgJLow>0 && nOrgJHigh < width-1)
			if(nOrgJLow>0 && nOrgJHigh < width-1)
            {
                pLineL1 = (unsigned char*)(pLineTmp + nOrgJLow*3);
                pLineH1 = (unsigned char*)(pLineTmp + nOrgJHigh*3);
				//pOut = (unsigned char*)(pOutTmp + nJ*3);

				nWeight1 = 1024-nDiff;
				nWeight2 = nDiff;
				//fprintf(fp,"%d,",nWeight1);
               
                nVal = (pLineL1[0]*nWeight1+pLineH1[0]*nWeight2)/1024;
                 pImgTmp[nCnt] =  clip(nVal);
                nVal = (pLineL1[1]*nWeight1+pLineH1[1]*nWeight2)/1024;
                pImgTmp[nCnt+1] =  clip(nVal);
                nVal = (pLineL1[2]*nWeight1+pLineH1[2]*nWeight2)/1024;
                pImgTmp[nCnt+2] =  clip(nVal);

            }
			else if(ReCalcFlag==1)
			{
				if(nJ < i_nOutWidth/2 && nJ > nLeftBound)
					nLeftBound = nJ;
				else if(nJ > i_nOutWidth/2 && nJ < nRightBound)
					nRightBound = nJ;

			}
        }
		//fprintf(fp,"\n");
    }
    

	nK = height-2; //��ʼ�ӵ����ڶ��п�ʼ 

    for(nI=i_nOutHeight-1;nI>=0;nI--)//����Ŵ�
    {

        nV = (((float)nI-((float)i_nOutHeight-height*g_f32ScaleRatio))/g_f32ScaleRatio)*1024;
        while(nV<g_nVertIdxShift[nK] && nK>0)
        {
            nK--;
        }

        nWeight1= g_nVertIdxShift[nK+1]-nV;
        nWeight2 = nV-g_nVertIdxShift[nK];
        nWeightSum = nWeight2+nWeight1;

        pOutTmp = (unsigned char*)(o_pImg+nOutWidthStep*nI);
        nCnt = nK*nOutWidthStep;
        nCntH = nCnt+nOutWidthStep;
	    if(nK>=0)
        for (nJ=0;nJ<i_nOutWidth;nJ++,nCnt+=3, nCntH+=3)//�������ڷǾ��ȷŴ�ʹ�����Բ�ֵ
        {
            pOut = (unsigned char*)(pOutTmp + nJ*3);
            nVal = (pImgTmp[nCntH]*nWeight2 + pImgTmp[nCnt]*nWeight1)/nWeightSum;
            pOut[0] = clip(nVal);
            nVal = (pImgTmp[nCntH+1]*nWeight2 + pImgTmp[nCnt+1]*nWeight1)/nWeightSum;
            pOut[1] = clip(nVal);
            nVal = (pImgTmp[nCntH+2]*nWeight2 + pImgTmp[nCnt+2]*nWeight1)/nWeightSum;
            pOut[2] = clip(nVal);

        }
    }

	if(ReCalcFlag==1)
	{
		pRect->x = nLeftBound;
		pRect->y = 0;
		pRect->width = (nRightBound-nLeftBound);
		pRect->height = i_nOutHeight;

	}
    return 0;
}


