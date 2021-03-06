#include "c_barrel_correction.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

//Debug
#include "debug_log.h"


CBarrelCorrection::CBarrelCorrection()
{
    // BarrelCorrection
    this->m_nBaseK0 = 1000;
    this->m_nBaseK1 = 100;

    this->m_fK0 = 300.0;
    this->m_fK1 = 0.0;

    this->m_nBarrelCorrectionFixedSizeFlag = 1;

    // VirtualStitch
    this->m_nVirtualStitchFlag = 0;  

    this->m_nZoom = 600;
    this->m_nBarrelUp = 8;
    this->m_nUpDownCurveRatio = 13;
    this->m_nUpDownRatio = 120;

    this->m_nFinalWidth = 1920;
    this->m_nFinalHeight = 1080;
}


CBarrelCorrection::~CBarrelCorrection()
{
}

void CBarrelCorrection::SetK0K1(float K0, float K1)
{
    this->m_fK0 = K0;
    this->m_fK1 = K1;
}

float CBarrelCorrection::GetK0()
{
    return this->m_fK0;
}

float CBarrelCorrection::GetK1()
{
    return this->m_fK1;
}

int BarrelCorrectProcess(unsigned char * i_pImg, int i_nWidth, int i_nHeight, int i_nNewWidth, int i_nNewHeight, int nHalfWidth, int nHalfHeight, float i_f32Ratio[2], unsigned char * o_pImg, CvRect * pRect, int ReCalcFlag, float scaledown, float scaledownH);
cv::Mat CBarrelCorrection::BarrelCorrection(cv::Mat matSrcImage)
{
    static float s_afK[2] = { -1.0, -1.0 };
    float fK[2];
    float fWratio, fHratio, fRatio;

    fK[0] = (float)(-(this->m_fK0 + this->m_nBaseK0) / 10000000000.0);
    fK[1] = (float)((this->m_fK1 + this->m_nBaseK1) / 10000000000000000.0);
    fWratio = fHratio = fRatio = 1.0;

    if (this->m_fK0 == 0 && this->m_fK1 == 0)
    {
        matSrcImage.copyTo(this->m_matBarrelCorrectionImage);
        this->m_nBarrelCorrectionWidth = this->m_matBarrelCorrectionImage.cols;
        this->m_nBarrelCorrectionHeight = this->m_matBarrelCorrectionImage.rows;
    }
    else
    {
        int nImageHalfWidth = matSrcImage.cols / 2;
        int nImageHalfHeight = matSrcImage.rows / 2;

        int nBarrelCorrectionValuesChangedFlag = 0;
        if (s_afK[0] != fK[0] || s_afK[1] != fK[0])
        {
            nBarrelCorrectionValuesChangedFlag = 1;

            int x, y;
            float x2_add_y2;
            int nNewImageHalfWidth, nNewImageHalfHeight;

            if (this->m_nBarrelCorrectionFixedSizeFlag)
            {
                this->m_nBarrelCorrectionWidth = this->m_nFinalWidth;
                this->m_nBarrelCorrectionHeight = this->m_nFinalHeight;

                x = nImageHalfWidth;
                y = nImageHalfHeight;

                // DebugMK
                cout << "(1.0 + fK[0] * x * x + fK[1] * x * x * x * x):" << (1.0 + fK[0] * x * x + fK[1] * x * x * x * x) << endl;
                cout << "(1.0 + fK[0] * y * y + fK[1] * y * y * y * y):" << (1.0 + fK[0] * y * y + fK[1] * y * y * y * y) << endl;

                nNewImageHalfWidth = x / (1.0 + fK[0] * x * x + fK[1] * x * x * x * x);
                nNewImageHalfHeight = y / (1.0 + fK[0] * y * y + fK[1] * y * y * y * y);

                fWratio = (float(this->m_nBarrelCorrectionWidth)) / (float(nNewImageHalfWidth * 2));
                fHratio = (float(this->m_nBarrelCorrectionHeight)) / (float(nNewImageHalfHeight * 2));

                // 选择需要需要缩放的最小倍率，另一个方向则裁剪（ratio越接近1，缩放倍率越小）
                if (fWratio < fHratio)
                {
                    fRatio = fHratio;
                }
                else
                {
                    fRatio = fWratio;
                }

                // DebugMK
                cout << "fK[0]:" << fK[0] << ", fK[1]:" << fK[1] << endl;
                cout << "fWratio:" << fWratio << ", fHratio:" << fHratio << ", fRatio:" << fRatio << endl;

                fWratio = fHratio = fRatio = 1;                
            }
            else
            {
                x = nImageHalfWidth;
                y = nImageHalfHeight;
                x2_add_y2 = x * x + y * y;

                // DebugMK
                cout << "(1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2):" << (1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2) << endl;

                nNewImageHalfWidth = fRatio * (x / (1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2));
                nNewImageHalfHeight = fRatio * (y / (1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2));

                this->m_nBarrelCorrectionWidth = nNewImageHalfWidth * 2;
                this->m_nBarrelCorrectionHeight = nNewImageHalfHeight * 2;
                
                fWratio = fHratio = fRatio;

                // DebugMK
                cout << "fK[0]:" << fK[0] << ", fK[1]:" << fK[1] << endl;
                cout << "fWratio:" << fWratio << ", fHratio:" << fHratio << ", fRatio:" << fRatio << endl;

            }

            // 更新变化值
            s_afK[0] = fK[0];
            s_afK[1] = fK[1];            
        }        

        // Execute BarrelCorrection algorithm
        if (nBarrelCorrectionValuesChangedFlag)
        {
            BYTE *pbySrcImage = new BYTE[matSrcImage.cols * matSrcImage.rows * 3];
            BYTE *pbyBarrelCorrectionImage = new BYTE[this->m_nBarrelCorrectionWidth * this->m_nBarrelCorrectionHeight * 3];

            this->m_cTransform.Mat2Array(matSrcImage, pbySrcImage);

            CvRect cvRect;
            BarrelCorrectProcess(pbySrcImage, matSrcImage.cols, matSrcImage.rows, this->m_nBarrelCorrectionWidth, this->m_nBarrelCorrectionHeight,
                nImageHalfWidth, nImageHalfHeight, fK, pbyBarrelCorrectionImage, &cvRect, nBarrelCorrectionValuesChangedFlag, fWratio, fHratio);

            this->m_matBarrelCorrectionImage = this->m_cTransform.Array2Mat(pbyBarrelCorrectionImage, this->m_nBarrelCorrectionWidth, this->m_nBarrelCorrectionHeight, 3);

            delete[] pbySrcImage;
            pbySrcImage = nullptr;
            delete[] pbyBarrelCorrectionImage;
            pbyBarrelCorrectionImage = nullptr;

            // DebugMK
            cout << "this->m_nBarrelCorrectionWidth:" << this->m_nBarrelCorrectionWidth << ", this->m_nBarrelCorrectionHeight:" << this->m_nBarrelCorrectionHeight << endl;
            cout << "cvRect.width:" << cvRect.width << ", cvRect.height:" << cvRect.height << endl;

            this->m_matBarrelCorrectionImage = this->m_matBarrelCorrectionImage(cvRect);
            this->m_nBarrelCorrectionWidth = this->m_matBarrelCorrectionImage.cols;
            this->m_nBarrelCorrectionHeight = this->m_matBarrelCorrectionImage.rows;

            nBarrelCorrectionValuesChangedFlag = 0;
        }
    }

    if (this->m_nVirtualStitchFlag)
    {
        return VirtualStitch(this->m_matBarrelCorrectionImage, fK);
    }

    return this->m_matBarrelCorrectionImage;
}

int VirtualStitchInit(int width, int height, float zoom, int endHeight, int startHeight, int *pOutWidth, int *pOutHeight, int level, int upMult, float max_ratio, int finalHeight);
int VirtualStitchProcess(unsigned char * i_pImg, int width, int height, unsigned char * o_pImg, int i_nOutWidth, int i_nOutHeight, CvRect * pRect, int ReCalcFlag, float i_f32Ratio[2], int i_nOrgWidth, int i_nOrgHeight);
int VirtualStitchRelease();
cv::Mat CBarrelCorrection::VirtualStitch(cv::Mat matSrcImage, float fK[])
{
    //m_nZoom = 600;
    //m_nBarrelUp = 8;
    //m_nUpDownCurveRatio = 13;
    //m_nUpDownRatio = 120;

    int nAdjustStartHeight, nAdjustEndHeight;
    nAdjustStartHeight = matSrcImage.rows / 4;
    nAdjustEndHeight = matSrcImage.rows - 1;

    float fZoomRatio;
    fZoomRatio = 1.0 + this->m_nZoom / 1000.0;

    float max_ratio = 0.9;

    static float s_fZoomRatio = -1.0;
    static int s_nAdjustStartHeight = 0, s_nAdjustEndHeight = 0, s_nBarrelUp = 0, s_nUpDownCurveRatio = 0, s_nUpDownRatio = 0;

    int nVirtualStitchValuesChangedFlag = 0;
    if (s_fZoomRatio != fZoomRatio || s_nAdjustStartHeight != nAdjustStartHeight || s_nAdjustEndHeight != nAdjustEndHeight 
        || s_nBarrelUp != this->m_nBarrelUp || s_nUpDownCurveRatio != this->m_nUpDownCurveRatio || s_nUpDownRatio != this->m_nUpDownRatio)
    {
        nVirtualStitchValuesChangedFlag = 1;

        max_ratio = (1000.0 - this->m_nUpDownRatio) / 1000.0;

        VirtualStitchInit(matSrcImage.cols, matSrcImage.rows, fZoomRatio, nAdjustEndHeight, nAdjustStartHeight, &(this->m_nVirtualStitchWidth), &(this->m_nVirtualStitchHeight), 
            this->m_nBarrelUp, this->m_nUpDownCurveRatio, max_ratio, this->m_nFinalHeight);

        // DebugMK
        cout << "this->m_nVirtualStitchWidth:" << this->m_nVirtualStitchWidth << ", this->m_nVirtualStitchHeight:" << this->m_nVirtualStitchHeight << ", this->m_nFinalHeight:" << this->m_nFinalHeight << endl;

        s_fZoomRatio = fZoomRatio;
        s_nAdjustStartHeight = nAdjustStartHeight;
        s_nAdjustEndHeight = nAdjustEndHeight;
        s_nBarrelUp = this->m_nBarrelUp;
        s_nUpDownCurveRatio = this->m_nUpDownCurveRatio;
        s_nUpDownRatio = this->m_nUpDownRatio;
    }

    // Execute VirtualStitch algorithm
    if (nVirtualStitchValuesChangedFlag)
    {
        BYTE *pbySrcImage = new BYTE[matSrcImage.cols * matSrcImage.rows * 3];
        BYTE *pbyVirtualStitchImage = new BYTE[this->m_nVirtualStitchWidth * this->m_nVirtualStitchHeight * 3];

        this->m_cTransform.Mat2Array(matSrcImage, pbySrcImage);

        CvRect cvRect;
        VirtualStitchProcess(pbySrcImage, matSrcImage.cols, matSrcImage.rows, pbyVirtualStitchImage, this->m_nVirtualStitchWidth, this->m_nVirtualStitchHeight, &cvRect, nVirtualStitchValuesChangedFlag, fK, this->m_nFinalWidth, this->m_nFinalHeight);

        this->m_matVirtualStitchImage = this->m_cTransform.Array2Mat(pbyVirtualStitchImage, this->m_nVirtualStitchWidth, this->m_nVirtualStitchHeight, 3);

        delete[] pbySrcImage;
        pbySrcImage = nullptr;
        delete[] pbyVirtualStitchImage;
        pbyVirtualStitchImage = nullptr;

        this->m_matVirtualStitchImage = this->m_matVirtualStitchImage(cvRect);

        VirtualStitchRelease();

        nVirtualStitchValuesChangedFlag = 0;
    }

    return this->m_matVirtualStitchImage;
}


cv::Mat CBarrelCorrection::BarrelCorrection_CPU(cv::Mat& matSrcImage)
{
    int SrcWidth = matSrcImage.cols;
    int SrcHeight = matSrcImage.rows;

    int inputWidth = SrcWidth;
    int inputHeight = SrcHeight;
    int inputPitchY = SrcWidth;
    int inputPitchUV = SrcWidth;

    int outputWidth = 1920;
    int outputHeight = 1080;
    int outputPitchY = 1920;
    int outputPitchUV = 1920;    

    unsigned char* pInputBuffer = (unsigned char*)malloc(inputPitchY * inputHeight * 3 / 2);
    unsigned char* pOutputBuffer = (unsigned char*)malloc(outputPitchY * outputHeight * 3 / 2);
    memset(pInputBuffer, 0, sizeof(unsigned char) * inputPitchY * inputHeight * 3 / 2);
    memset(pOutputBuffer, 0, sizeof(unsigned char) * outputPitchY * outputHeight * 3 / 2);
    if ((NULL == pInputBuffer) || (NULL == pOutputBuffer))
    {
        printf("malloc failed!!!\n");
    }

    // 生成300W yuv图像
    Mat2NV12(matSrcImage, pInputBuffer, pInputBuffer + inputPitchY * inputHeight, inputWidth, inputHeight);
    imshowNV12(pInputBuffer, pInputBuffer + inputPitchY * inputHeight, inputWidth, inputHeight);
    imwriteNV12(pInputBuffer, pInputBuffer + inputPitchY * inputHeight, inputWidth, inputHeight, "./Resource/srcYUVN12.yuv");


    EMISPAlgType emISPAlgType = BARRELCORRECTION;
    TISPVersionInfo tISPVersionInfo;
    void* pAlgHandle = NULL;
    void* ptOpen = NULL;        

    TBarrelCorrectionOpen tBarrelCorrectionOpen;

    tBarrelCorrectionOpen.flag = 0;
    tBarrelCorrectionOpen.tBCOpen.emFormat = IMGALG_NV12;
    tBarrelCorrectionOpen.tBCOpen.u32InputWidth = inputWidth;
    tBarrelCorrectionOpen.tBCOpen.u32InputHeight = inputHeight;
    tBarrelCorrectionOpen.tBCOpen.u32InputPitchY = inputPitchY;
    tBarrelCorrectionOpen.tBCOpen.u32InputPitchUV = inputPitchUV;
    tBarrelCorrectionOpen.tBCOpen.u32OutputWidth = outputWidth;
    tBarrelCorrectionOpen.tBCOpen.u32OutputHeight = outputHeight;

    tBarrelCorrectionOpen.tBCOpen.u32Ratio1 = int(this->m_fK0 + this->m_nBaseK0);
    tBarrelCorrectionOpen.tBCOpen.u32Ratio2 = int(this->m_fK1 + this->m_nBaseK1);
    //////////////////////////////////////////////////////////////////////////
    tBarrelCorrectionOpen.tBCOpen.flagROIMode = 0;
    tBarrelCorrectionOpen.tBCOpen.f32w = 0.8;
    tBarrelCorrectionOpen.tBCOpen.f32h = 0.6;
    tBarrelCorrectionOpen.tBCOpen.f32x0 = 0;
    tBarrelCorrectionOpen.tBCOpen.f32y0 = 0;
    //////////////////////////////////////////////////////////////////////////
    ptOpen = (void*)(&tBarrelCorrectionOpen);
        

    if (ISPVideoAlgInit() != SUCCESS_GPUALG)
    {
        printf("init error!\n");
        free(pInputBuffer);
        free(pOutputBuffer);
        return matSrcImage;
    }

    GetISPVersionInfo(emISPAlgType, &tISPVersionInfo);

    printf("=========>begin to open!\n");
    if (ISPVideoAlgOpen(&pAlgHandle, emISPAlgType, ptOpen) != SUCCESS_GPUALG)
    {
        printf("open error!\n");
        ISPVideoAlgRelease();
        free(pInputBuffer);
        free(pOutputBuffer);
        return matSrcImage;
    }

    TISPImageInfo tInputISPImageInfo;
    TISPImageInfo tOutputISPImgeInfo;

    tInputISPImageInfo.tImageBuffer[0].pu8ImageDataY = pInputBuffer;
    tInputISPImageInfo.tImageBuffer[0].pu8ImageDataU = pInputBuffer + inputPitchY * inputHeight;
    tInputISPImageInfo.tImageBuffer[0].emFormat = IMGALG_NV12;
    tInputISPImageInfo.tImageBuffer[0].u32Width = inputWidth;
    tInputISPImageInfo.tImageBuffer[0].u32Height = inputHeight;
    tInputISPImageInfo.tImageBuffer[0].u32PitchY = inputWidth;
    tInputISPImageInfo.tImageBuffer[0].u32PitchUV = inputWidth;
    tInputISPImageInfo.pvImageInfo = NULL;

    tOutputISPImgeInfo.tImageBuffer[0].pu8ImageDataY = pOutputBuffer;
    tOutputISPImgeInfo.tImageBuffer[0].pu8ImageDataU = pOutputBuffer + outputPitchY * outputHeight;
    tOutputISPImgeInfo.tImageBuffer[0].u32Width = outputWidth;
    tOutputISPImgeInfo.tImageBuffer[0].u32Height = outputHeight;
    tOutputISPImgeInfo.tImageBuffer[0].u32PitchY = outputPitchY;
    tOutputISPImgeInfo.tImageBuffer[0].u32PitchUV = outputPitchUV;


    printf("=========>begin to fread!\n");
    imreadNV12(pInputBuffer, pInputBuffer + inputPitchY * inputHeight, inputPitchY, inputHeight, "./Resource/srcYUVN12.yuv");
    imshowNV12(pInputBuffer, pInputBuffer + inputPitchY * inputHeight, inputWidth, inputHeight);    


    printf("=========>begin to process!\n");
    if (ISPVideoAlgProcess(pAlgHandle, emISPAlgType, &tInputISPImageInfo, &tOutputISPImgeInfo) != SUCCESS_GPUALG)
    {
        printf("ISPVideoAlgProcess error!\n");
        ISPVideoAlgClose(pAlgHandle, emISPAlgType);
        ISPVideoAlgRelease();
        free(pInputBuffer);
        free(pOutputBuffer);
        return matSrcImage;
    }

    printf("=========>begin to close!\n");
    ISPVideoAlgClose(pAlgHandle, emISPAlgType);
    ISPVideoAlgRelease();


    // 生产YUV文件
    imwriteNV12(tOutputISPImgeInfo.tImageBuffer[0].pu8ImageDataY, tOutputISPImgeInfo.tImageBuffer[0].pu8ImageDataU, 
        tOutputISPImgeInfo.tImageBuffer[0].u32Width, tOutputISPImgeInfo.tImageBuffer[0].u32Height,
        "./Resource/tmp.yuv");    

    cv::Mat matResultImage = imshowNV12(pOutputBuffer, pOutputBuffer + outputPitchY * outputHeight, outputWidth, outputHeight);

    // 释放资源
    free(pInputBuffer);
    free(pOutputBuffer);

    return matResultImage;
}


void CBarrelCorrection::imreadNV12(unsigned char* pu8Y, unsigned char* pu8UV, int nWidth, int nHeight, char *pchFilePath)
{
    FILE* pfInputFileOpen = NULL;
    pfInputFileOpen = fopen(pchFilePath, "rb");
    if (NULL == pfInputFileOpen)
    {
        printf("%s: Couldn't open file: %s\n", __FUNCTION__, pchFilePath);
    }

    printf("u32Height = %d, u32PitchY = %d, u32Width = %d\n", nHeight, nWidth, nWidth);
    fread(pu8Y, 1, nWidth * nHeight * 3 / 2, pfInputFileOpen);

    fclose(pfInputFileOpen);
}


cv::Mat CBarrelCorrection::imshowNV12(unsigned char* pu8Y, unsigned char* pu8UV, int nWidth, int nHeight)
{
    // NV12--->YUV
    unsigned char* pYUVBufResult = NULL;
    int ResultWidth = nWidth;
    int ResultHeight = nHeight;
    pYUVBufResult = (unsigned char*)malloc(ResultWidth * ResultHeight * 3 / 2 * sizeof(unsigned char));

    unsigned char* pYUVBufTmp = NULL;
    unsigned char* pYBufTmp = NULL;
    unsigned char* pUVBufTmp = NULL;
    
    pYUVBufTmp = pYUVBufResult;
    pYBufTmp = pu8Y;
    pUVBufTmp = pu8UV;

    for (int i = 0; i < ResultWidth * ResultHeight * 5 / 4; i++)
    {
        if (i < ResultWidth * ResultHeight)
        {
            *pYUVBufTmp = *pYBufTmp;   // Y
            pYBufTmp++;
            pYUVBufTmp++;
        }
        else
        {
            *pYUVBufTmp = *pUVBufTmp;   //U
            pUVBufTmp++;
            *(pYUVBufTmp + ResultWidth * ResultHeight / 4) = *pUVBufTmp;   // V
            pUVBufTmp++;

            pYUVBufTmp++;
        }
    }

    cv::Mat yuvImgResult;
    yuvImgResult.create(ResultHeight * 3 / 2, ResultWidth, CV_8UC1);
    memcpy(yuvImgResult.data, pYUVBufResult, ResultWidth * ResultHeight * 3 / 2 * sizeof(unsigned char));

    // YUV--->RGB
    cv::Mat rgbImgResult;
    cv::cvtColor(yuvImgResult, rgbImgResult, CV_YUV2BGR_I420);

    // 释放资源
    free(pYUVBufResult);
    pYUVBufResult = NULL;

    pYUVBufTmp = NULL;
    pYBufTmp = NULL;
    pUVBufTmp = NULL;

    return rgbImgResult;
}                                           


void CBarrelCorrection::imwriteNV12(unsigned char* pu8Y, unsigned char* pu8UV, int nWidth, int nHeight, char *pchFilePath)
{
    FILE* pfOutputFileOpen = NULL;
    pfOutputFileOpen = fopen(pchFilePath, "wb");
    if (NULL == pfOutputFileOpen)
    {
        printf("%s: Couldn't open file: %s\n", __FUNCTION__, pchFilePath);
    }

    printf("u32Height = %d, u32PitchY = %d, u32Width = %d\n", nHeight, nWidth, nWidth);
    for (int k = 0; k < nHeight; k++)
    {
        fwrite(pu8Y + k*nWidth, 1, nWidth, pfOutputFileOpen);
    }

    for (int k = 0; k < nHeight / 2; k++)
    {
        fwrite(pu8UV + k*nWidth, 1, nWidth, pfOutputFileOpen);
    }

    fclose(pfOutputFileOpen);
}


void CBarrelCorrection::Mat2NV12(cv::Mat matSrcImage, unsigned char *pu8Y, unsigned char *pu8UV, int nWidth, int nHeight)
{
    cv::Mat yuvImgResult;
    yuvImgResult.create(nHeight * 3 / 2, nWidth, CV_8UC1);    

    // Mat--->YUV
    cv::cvtColor(matSrcImage, yuvImgResult, CV_BGR2YUV_I420);
    
    unsigned char* pYUVBuf = NULL;
    pYUVBuf = (unsigned char*)malloc(nWidth * nHeight * 3 / 2 * sizeof(unsigned char));
    memcpy(pYUVBuf, yuvImgResult.data, nWidth * nHeight * 3 / 2 * sizeof(unsigned char));

    // YUV--->NV12
    unsigned char* pYUVBufTmp = NULL;
    unsigned char* pYBufTmp = NULL;
    unsigned char* pUVBufTmp = NULL;

    pYUVBufTmp = pYUVBuf;
    pYBufTmp = pu8Y;
    pUVBufTmp = pu8UV;

    for (int i = 0; i < nWidth * nHeight * 5 / 4; i++)
    {
        if (i < nWidth * nHeight)
        {
            *pYBufTmp = *pYUVBufTmp;
            pYBufTmp++;
            pYUVBufTmp++;
        }
        else
        {
            *pUVBufTmp = *pYUVBufTmp;
            pUVBufTmp++;
            *pUVBufTmp = *(pYUVBufTmp + nWidth * nHeight / 4);
            pUVBufTmp++;

            pYUVBufTmp++;
        }
    }

    // 释放资源
    free(pYUVBuf);
    pYUVBuf = NULL;

    pYUVBufTmp = NULL;
    pYBufTmp = NULL;
    pUVBufTmp = NULL;
}
