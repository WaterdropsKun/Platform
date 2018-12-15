#include "MainForm.h"

#include <QFileDialog>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <string>
using namespace std;

#include "c_show_thread.h"

//Debug
#include "debug_log.h"
#include "c_transform.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);


    this->Init();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::Open()
{
    QString qstrFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择文件"));
    if (qstrFileName.isEmpty())
    {
        return;
    }
    string strFileName = qstrFileName.toLocal8Bit().data();

    if (!CShowThread::Get()->Open(strFileName))
    {
        // DebugMK
        QMessageBox::information(this, QStringLiteral("Debug"), qstrFileName);
    }
    
}

void MainWindow::ValueChanged()
{
    ui.nCut_le->setText(QString::number(ui.nCut_hs->value()));
    ui.nBarrel_ParaL_le->setText(QString::number(ui.nBarrel_ParaL_hs->value()));
    ui.nBarrel_ParaS_le->setText(QString::number(ui.nBarrel_ParaS_hs->value()));

    int nCut = ui.nCut_hs->value();
    int nBarrel_ParaL = ui.nBarrel_ParaL_hs->value();
    int nBarrel_ParaS = ui.nBarrel_ParaS_hs->value();

    this->BarrelCorrection(nCut, nBarrel_ParaL, nBarrel_ParaS);
}


void MainWindow::Init()
{
    qRegisterMetaType<cv::Mat>("cv::Mat");
    QObject::connect(CShowThread::Get(),
        SIGNAL(SendMat(cv::Mat)),
        ui.ImageShow,
        SLOT(ReceiveMat(cv::Mat))
        );

    // DebugMK
    this->Test();
}

void MainWindow::Test()
{
    //this->HistogramEnhancemen();

    //this->DistortionCorrection(100, 1, 1);
}


int BarrelCorrectProcess(unsigned char * i_pImg, int i_nWidth, int i_nHeight, int i_nNewWidth, int i_nNewHeight, int nHalfWidth, int nHalfHeight, float i_f32Ratio[2], unsigned char * o_pImg, CvRect * pRect, int ReCalcFlag, float scaledown, float scaledownH);
int VirtualStitchInit(int width, int height, float zoom, int endHeight, int startHeight, int *pOutWidth, int *pOutHeight, int level, int upMult, float max_ratio, int finalHeight);
int VirtualStitchRelease();
int VirtualStitchProcess(unsigned char * i_pImg, int width, int height, unsigned char * o_pImg, int i_nOutWidth, int i_nOutHeight, CvRect * pRect, int ReCalcFlag, float i_f32Ratio[2], int i_nOrgWidth, int i_nOrgHeight);
void MainWindow::BarrelCorrection(int nCut_, int nBarrelK0_, int nBarrelK1_)
{
    // 对话框获取文件路径
    //string strFileName;
    //QString qstrFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择文件"));
    //if (qstrFileName.isEmpty())
    //{
    //    return;
    //}
    //strFileName = qstrFileName.toLocal8Bit().data();
    //printf("strFileName: %s\n", strFileName.c_str());

    // 显示原始图像
    //cv::Mat matSrcImage = cv::imread(strFileName);
    cv::Mat matSrcImage = cv::imread("./Resource/矫正前.jpeg");
    if (matSrcImage.empty())
    {
        cout << "Could not load matSrcImage!" << endl;
    }
    cv::Mat matSrcImageShow;   // 原始图像--->裁剪
    cv::resize(matSrcImage, matSrcImageShow, cv::Size(matSrcImage.cols / 4, matSrcImage.rows / 4));
    cv::imshow("matSrcImageShow", matSrcImageShow);

    // Mat和Array相互转换类
    CTransform cTransform;
    // VirtualStitch使能标志位
    int nVirtualStitchFlag = 0;

    // 固定输入参数
    int nBASEK0 = 1600;
    int nBASEK1 = 160;

    int nHalfWidth = matSrcImage.cols / 2;
    int nHalfHeight = matSrcImage.rows / 2;

    int nIsBarrelSizeFlag = 1;   // 是否按设定大小裁剪标志位
    int nFinalWidth = matSrcImage.cols;
    int nFinalHeight = matSrcImage.rows;

    // 可调输入参数
    int nCut = nCut_;
    int nBarrelK0 = nBarrelK0_;
    int nBarrelK1 = nBarrelK1_;

    // 是否计算输出图片尺寸标志位
    int nIsReCalcBarrelFlag = 1;
    cv::Mat matBarrelImage;
    cv::Mat matBarrelImageROI;    // 结果图像--->裁剪
    cv::Mat matBarrelImageShow;   // 裁剪--->缩放


    int x, y;
    float x2_add_y2;
    int nNewHalfWidth, nNewHalfHeight;
    int nBarrelWidth, nBarrelHeight;


    static int s_nCut = -1;
    static float s_afK[2] = { -1.0, -1.0 };    
    static float fWratio = 1.0, fHratio = 1.0;

    float fK[2];
    float fRatio;


    fK[0] = (float)(-(nBarrelK0 + nBASEK0) / 10000000000.0);
    fK[1] = (float)((nBarrelK1 + nBASEK1) /  10000000000000000.0);
    fRatio = 1.0;

    if (nBarrelK1 == 0 && nBarrelK0 == 0)
    {
        matSrcImage.copyTo(matBarrelImage);
        nBarrelWidth = matSrcImage.cols;
        nBarrelHeight = matSrcImage.rows;
    }
    else
    {
        if (s_afK[0] != fK[0] || s_afK[1] != fK[0] || s_nCut != nCut)
        {
            // 使能VirtualStitch标志位
            nVirtualStitchFlag = 1;

            // 一旦改变了畸变参数，重新计算输出图片尺寸
            nIsReCalcBarrelFlag = 1;

            if (nIsBarrelSizeFlag)
            {
                x = nHalfWidth;
                y = nHalfHeight;
                nNewHalfWidth = x / (1.0 + fK[0] * x * x + fK[1] * x * x * x * x);                
                nNewHalfHeight = y / (1.0 + fK[0] * y * y + fK[1] * y * y * y * y);

                //fWratio = (float(nFinalWidth)) / (float(nNewHalfWidth / 4) * 8);
                fWratio = (float(nFinalWidth)) / (float(nNewHalfWidth * 2));
                fHratio = (float(nFinalHeight)) / (float(nNewHalfHeight * 2));

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
                fWratio = fHratio = fRatio;

                nBarrelWidth = nFinalWidth;
                nBarrelHeight = nFinalHeight;
            }
            else
            {
                x = nHalfWidth;
                y = nHalfHeight;
                x2_add_y2 = x * x + y * y;
                float mk = (1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2);
                nNewHalfWidth = fRatio * (x / (1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2));
                nNewHalfHeight = fRatio * (y / (1.0 + fK[0] * x2_add_y2 + fK[1] * x2_add_y2 * x2_add_y2));

                //nBarrelWidth = (nNewHalfWidth / 4) * 8;
                nBarrelWidth = nNewHalfWidth * 2;
                nBarrelHeight = nNewHalfHeight * 2;

                fWratio = fHratio = fRatio * 100.0 / (float)nCut;
            }            

            // 更新变化值
            s_afK[0] = fK[0];
            s_afK[1] = fK[1];            
            s_nCut = nCut;
        }  

        BYTE *pbySrcImage = new BYTE[matSrcImage.rows * matSrcImage.cols * 3];
        BYTE *pbyDesImage = new BYTE[nBarrelWidth * nBarrelHeight * 3];

        cTransform.Mat2Array(matSrcImage, pbySrcImage);

        CvRect cvRect;
        BarrelCorrectProcess(pbySrcImage, matSrcImage.cols, matSrcImage.rows, nBarrelWidth, nBarrelHeight, nHalfWidth, nHalfHeight, fK, pbyDesImage, &cvRect, nIsReCalcBarrelFlag, fWratio, fHratio);

        matBarrelImage = cTransform.Array2Mat(pbyDesImage, nBarrelWidth, nBarrelHeight, 3);

        delete[] pbySrcImage;
        pbySrcImage = nullptr;
        delete[] pbyDesImage;
        pbyDesImage = nullptr;

        if (nIsReCalcBarrelFlag)
        {            
            nBarrelWidth = cvRect.width;
            nBarrelHeight = cvRect.height;

            matBarrelImageROI = matBarrelImage(cvRect);
            cv::resize(matBarrelImageROI, matBarrelImageShow, cv::Size(nBarrelWidth / 4, nBarrelHeight / 4));
            cv::imshow("matDesImageShow", matBarrelImageShow);

            nIsReCalcBarrelFlag = 0;
        }       
    }


    ////////////////////////////////////////////////////////////////////////////////    
    if (nVirtualStitchFlag)
    {
        // 可调输入参数
        int nZoom = 600;
        int nBarrelUp = 8;
        int nUpDownCurveRatio = 13;
        int nUpDownRatio = 120;
            
        int nIsReCalcVSFlag = 0;
        cv::Mat matVSImage;
        cv::Mat matVSImageROI;
        cv::Mat matVSImageShow;

        int nVSWidth, nVSHeight;
        
        int nAdjustStartHeight, nAdjustEndHeight;
        nAdjustStartHeight = nBarrelHeight / 4;
        nAdjustEndHeight = nBarrelHeight - 1;

        float fZoomRatio;
        fZoomRatio = 1.0 + nZoom / 1000.0;

        float max_ratio = 0.9;


        static float s_fZoomRatio;
        static int s_nAdjustStartHeight, s_nAdjustEndHeight, s_nBarrelUp, s_nUpDownCurveRatio, s_nUpDownRatio;

        /*if (s_fZoomRatio != fZoomRatio || s_nAdjustStartHeight != nAdjustStartHeight || s_nAdjustEndHeight != nAdjustEndHeight
            || s_nBarrelUp != nBarrelUp || s_nUpDownCurveRatio != nUpDownCurveRatio || s_nUpDownRatio != nUpDownRatio)*/
        {
            nIsReCalcVSFlag = 1;

            max_ratio = (1000.0 - nUpDownRatio) / 1000.0;

            VirtualStitchInit(nBarrelWidth, nBarrelHeight, fZoomRatio, nAdjustEndHeight, nAdjustStartHeight, &nVSWidth, &nVSHeight, nBarrelUp, nUpDownCurveRatio, max_ratio, nFinalHeight);

            s_fZoomRatio = fZoomRatio;
            s_nAdjustStartHeight = nAdjustStartHeight;
            s_nAdjustEndHeight = nAdjustEndHeight;
            s_nBarrelUp = nBarrelUp;
            s_nUpDownCurveRatio = nUpDownCurveRatio;
            s_nUpDownRatio = nUpDownRatio;
        }

        BYTE *pbyDesImageROI = new BYTE[matBarrelImageROI.cols * matBarrelImageROI.rows * 3];
        BYTE *pbyVSImage = new BYTE[nVSWidth * nVSHeight * 3];
       
        cTransform.Mat2Array(matBarrelImageROI, pbyDesImageROI);

        CvRect cvRect;
        VirtualStitchProcess(pbyDesImageROI, nBarrelWidth, nBarrelHeight, pbyVSImage, nVSWidth, nVSHeight, &cvRect, nIsReCalcVSFlag, fK, matSrcImage.cols, matSrcImage.rows);

        matVSImage = cTransform.Array2Mat(pbyVSImage, nVSWidth, nVSHeight, 3);

        delete[] pbyDesImageROI;
        pbyDesImageROI = nullptr;
        delete[] pbyVSImage;
        pbyVSImage = nullptr;

        if (nIsReCalcVSFlag == 1)
        {
            matVSImageROI = matVSImage(cvRect);

            cv::resize(matVSImageROI, matVSImageShow, cv::Size(matVSImageROI.cols / 4, matVSImageROI.rows / 4));
            cv::imshow("matVSImageShow", matVSImageShow);

            nIsReCalcVSFlag = 0;
        }

        VirtualStitchRelease();
    }
}


extern "C" void AMHE_main_run(int *img, int m, int n, double c, int binNum, double *newHistOut, int *Map);
void MainWindow::HistogramEnhancemen()
{
    // 输入对比度
    double dContrast = 1.0;
    printf("请输入对比度参数，增强时范围为[0,1.5]，模糊范围为[-1,0]，最优参数为1:");
    scanf("%lf", &dContrast);
    cout << "dContrast: " << dContrast << endl;

    // 对话框获取文件路径
    string strFileName;
    QString qstrFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择文件"));
    if (qstrFileName.isEmpty())
    {
        return;
    }
    strFileName = qstrFileName.toLocal8Bit().data();
    printf("strFileName: %s\n", strFileName.c_str());

    // 显示原始图像
    cv::Mat matSrcImage = cv::imread(strFileName);
    if (matSrcImage.empty())
    {
        cout << "Could not load matSrcImage!" << endl;
    }
    cv::imshow("matSrcImage", matSrcImage);

    // 提取图像R, G, B通道数据，并转成CV_32FC1格式，计算图像亮度成分
    cv::Mat matImageBlueChannel;
    cv::Mat matImageGreenChannel;
    cv::Mat matImageRedChannel;
    cv::Mat matBright(matSrcImage.rows, matSrcImage.cols, CV_32FC1);
    cv::Mat matBrightTmp(matSrcImage.rows, matSrcImage.cols, CV_8UC1);

    std::vector<cv::Mat> vChannels;
    cv::split(matSrcImage, vChannels);
    vChannels[0].convertTo(matImageBlueChannel, CV_32FC1, 1.0, 0.0);
    vChannels[1].convertTo(matImageGreenChannel, CV_32FC1, 1.0, 0.0);
    vChannels[2].convertTo(matImageRedChannel, CV_32FC1, 1.0, 0.0);

    matBright = 0.299 * matImageRedChannel + 0.587 * matImageGreenChannel + 0.114 * matImageBlueChannel;
    matBright.convertTo(matBrightTmp, CV_8UC1);
    cv::imwrite("./Resource/matBrightTmp.jpeg", matBrightTmp);

    // Mat--->Array：分配输入图像空间，并把亮度图像输入图像空间
    int *pnInputImg = (int *)malloc(sizeof(int)*(matSrcImage.cols)*(matSrcImage.rows));
    if (NULL == pnInputImg)
    {
        cout << "Could not malloc pnInputImg!" << endl;
    }
    Mat2Array(matBright, pnInputImg);

    // 分配映射表空间，运行图像增强算法，计算图像映射表
    int nBinNum = 256;
    int *pMap = (int *)malloc(sizeof(int) * nBinNum);
    double *pdNewHistOut = (double *)malloc(sizeof(double) * nBinNum);

    AMHE_main_run(pnInputImg, matSrcImage.cols, matSrcImage.rows, dContrast, nBinNum, pdNewHistOut, pMap);

    // Array--->Mat：根据映射表得到输出图像目标亮度
    cv::Mat matOutputImg(matSrcImage.rows, matSrcImage.cols, CV_8UC1);
    for (int i = 0; i < matSrcImage.rows; i++)
    {
        for (int j = 0; j < matSrcImage.cols; j++)
        {
            matOutputImg.at<uchar>(i, j) = (uchar)(pMap[pnInputImg[i * matSrcImage.cols + j]]);
        }
    }
    cv::imwrite("./Resource/matOutputImg.jpeg", matOutputImg);

    // 三通道融合成彩色图像
    cv::Mat matDesImg(matSrcImage.rows, matSrcImage.cols, CV_32FC3);

    double r, g, b;
    for (int i = 0; i < matSrcImage.rows; i++)
    {
        for (int j = 0; j < matSrcImage.cols; j++)
        {
            if (matBright.at<float>(i, j) == 0)
            {
                matBright.at<float>(i, j) = 1;
            }

            r = matImageRedChannel.at<float>(i, j) * matOutputImg.at<uchar>(i, j) / matBright.at<float>(i, j);
            g = matImageGreenChannel.at<float>(i, j) * matOutputImg.at<uchar>(i, j) / matBright.at<float>(i, j);
            b = matImageBlueChannel.at<float>(i, j) * matOutputImg.at<uchar>(i, j) / matBright.at<float>(i, j);

            matDesImg.at<cv::Vec3f>(i, j) = cv::Vec3f(b, g, r);
        }
    }

    // 显示结果图像
    cv::Mat matResultImg;
    matDesImg.convertTo(matResultImg, CV_8UC3);
    if (matResultImg.empty())
    {
        cout << "Could not load matResultImg!" << endl;
    }
    cv::imshow("matResultImg", matResultImg);

    cv::imwrite("./Resource/ResultImg.jpeg", matResultImg);


    free(pnInputImg);
    pnInputImg = NULL;
}


// 两处需要修改：
// 1、int *pArray
// 2、int *pbyImgeRgb = pArray;
void MainWindow::Mat2Array(cv::Mat matImage, int *pArray)
{
    if (matImage.empty() || pArray == NULL)
    {
        return;
    }

    // 没有填充像素，把行设为1，列设为H*W，从而去除外层的循环	
    int nr = matImage.rows;
    int nc = matImage.cols * matImage.channels();
    if (matImage.isContinuous())
    {
        nc = nr * nc;
        nr = 1;
    }

    int *pbyImgeRgb = pArray;
    for (int i = 0; i < nr; i++)
    {
        cout << "matImage.type(): " << matImage.type() << endl;
        // 输入图像类型是CV_32F
        if (CV_32F == matImage.type())
        {
            float *data = matImage.ptr<float>(i);

            for (int j = 0; j < nc; j++)
            {
                *pbyImgeRgb = (uchar)data[j];
                pbyImgeRgb++;
            }
        }
        // 输入图像类型是CV_8U
        else if (CV_8UC3 == matImage.type())
        {
            BYTE* data = matImage.ptr<uchar>(i);

            for (int j = 0; j < nc; j++)
            {
                *pbyImgeRgb = (uchar)data[j];
                pbyImgeRgb++;
            }
        }        
    }
}






