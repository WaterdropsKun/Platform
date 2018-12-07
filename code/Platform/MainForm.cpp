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
    QString qstrFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ���ļ�"));
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

    this->DistortionCorrection(nCut, nBarrel_ParaL, nBarrel_ParaS);
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
void MainWindow::DistortionCorrection(int nCut_, int nBarrel_ParaL_, int nBarrel_ParaS_)
{
    // �Ի����ȡ�ļ�·��
    //string strFileName;
    //QString qstrFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ���ļ�"));
    //if (qstrFileName.isEmpty())
    //{
    //    return;
    //}
    //strFileName = qstrFileName.toLocal8Bit().data();
    //printf("strFileName: %s\n", strFileName.c_str());

    // ��ʾԭʼͼ��
    //cv::Mat matSrcImage = cv::imread(strFileName);
    cv::Mat matSrcImage = cv::imread("./Resource/����ǰ.jpeg");
    if (matSrcImage.empty())
    {
        cout << "Could not load matSrcImage!" << endl;
    }
    cv::Mat matSrcImageShow;   // ԭʼͼ��--->�ü�
    cv::resize(matSrcImage, matSrcImageShow, cv::Size(matSrcImage.cols / 4, matSrcImage.rows / 4));
    cv::imshow("matSrcImageShow", matSrcImageShow);

    // Mat��Array�໥ת����
    CTransform cTransform;

    // �̶��������
    int nBASE1 = 1600;
    int nBASE3 = 160;

    int nHalfWidth = matSrcImage.cols / 2;
    int nHalfHeight = matSrcImage.rows / 2;

    int nIsBarrelSizeFlag = 0;   // �Ƿ��趨��С�ü���־λ
    int nFinalWidth = matSrcImage.cols;
    int nFinalHeight = matSrcImage.rows;

    // �ɵ��������
    int nCut = nCut_;
    int nBarrel_ParaL = nBarrel_ParaL_;
    int nBarrel_ParaS = nBarrel_ParaS_;


    cv::Mat matDesImage;
    cv::Mat matDesImageROI;   // ���ͼ��--->�ü�
    cv::Mat matDesImageShow;   // �ü�--->����


    int x, y;
    float x2_add_y2;
    int nNewHalfWidth, nNewHalfHeight;
    int nBarrelWidth, nBarrelHeight;

    // �Ƿ����²ü���־λ
    int nIsReCutFlag = 1;   

    static int s_nCut = -1;
    static float s_afK[2] = { -1.0, -1.0 };    
    static float fWratio = 1.0, fHratio = 1.0;

    float fK[2];
    float fRatio;


    fK[0] = (float)(-(nBarrel_ParaL + nBASE1) / 10000000000.0);
    fK[1] = (float)((nBarrel_ParaS + nBASE3) /  10000000000000000.0);
    fRatio = 1.0;

    if (nBarrel_ParaS == 0 && nBarrel_ParaL == 0)
    {
        matSrcImage.copyTo(matDesImage);
        nBarrelWidth = matSrcImage.cols;
        nBarrelHeight = matSrcImage.rows;
    }
    else
    {
        if (s_afK[0] != fK[0] || s_afK[1] != fK[0] || s_nCut != nCut)
        {
            // һ���ı��˻���������趨�Ƿ����²ü���־λ
            nIsReCutFlag = 1;

            if (nIsBarrelSizeFlag)
            {
                x = nHalfWidth;
                y = nHalfHeight;
                nNewHalfWidth = x / (1.0 + fK[0] * x * x + fK[1] * x * x * x * x);                
                nNewHalfHeight = y / (1.0 + fK[0] * y * y + fK[1] * y * y * y * y);

                //fWratio = (float(nFinalWidth)) / (float(nNewHalfWidth / 4) * 8);
                fWratio = (float(nFinalWidth)) / (float(nNewHalfWidth * 2));
                fHratio = (float(nFinalHeight)) / (float(nNewHalfHeight * 2));

                // ѡ����Ҫ��Ҫ���ŵ���С���ʣ���һ��������ü���ratioԽ�ӽ�1�����ű���ԽС��
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

            // ���±仯ֵ
            s_afK[0] = fK[0];
            s_afK[1] = fK[1];            
            s_nCut = nCut;
        }  

        BYTE *pbySrcImage = new BYTE[matSrcImage.rows * matSrcImage.cols * 3];
        BYTE *pbyDesImage = new BYTE[nBarrelWidth * nBarrelHeight * 3];

        cTransform.Mat2Array(matSrcImage, pbySrcImage);

        CvRect cvRect;
        BarrelCorrectProcess(pbySrcImage, matSrcImage.cols, matSrcImage.rows, nBarrelWidth, nBarrelHeight, nHalfWidth, nHalfHeight, fK, pbyDesImage, &cvRect, nIsReCutFlag, fWratio, fHratio);

        matDesImage = cTransform.Array2Mat(pbyDesImage, nBarrelWidth, nBarrelHeight, 3);

        delete[] pbySrcImage;
        pbySrcImage = nullptr;
        delete[] pbyDesImage;
        pbyDesImage = nullptr;

        if (nIsReCutFlag)
        {            
            nBarrelWidth = cvRect.width;
            nBarrelHeight = cvRect.height;

            matDesImageROI = matDesImage(cvRect);
            cv::resize(matDesImageROI, matDesImageShow, cv::Size(nBarrelWidth / 4, nBarrelHeight / 4));
            cv::imshow("matDesImageShow", matDesImageShow);

            nIsReCutFlag = 0;
        }       
    }
}


extern "C" void AMHE_main_run(int *img, int m, int n, double c, int binNum, double *newHistOut, int *Map);
void MainWindow::HistogramEnhancemen()
{
    // ����Աȶ�
    double dContrast = 1.0;
    printf("������ԱȶȲ�������ǿʱ��ΧΪ[0,1.5]��ģ����ΧΪ[-1,0]�����Ų���Ϊ1:");
    scanf("%lf", &dContrast);
    cout << "dContrast: " << dContrast << endl;

    // �Ի����ȡ�ļ�·��
    string strFileName;
    QString qstrFileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("��ѡ���ļ�"));
    if (qstrFileName.isEmpty())
    {
        return;
    }
    strFileName = qstrFileName.toLocal8Bit().data();
    printf("strFileName: %s\n", strFileName.c_str());

    // ��ʾԭʼͼ��
    cv::Mat matSrcImage = cv::imread(strFileName);
    if (matSrcImage.empty())
    {
        cout << "Could not load matSrcImage!" << endl;
    }
    cv::imshow("matSrcImage", matSrcImage);

    // ��ȡͼ��R, G, Bͨ�����ݣ���ת��CV_32FC1��ʽ������ͼ�����ȳɷ�
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

    // Mat--->Array����������ͼ��ռ䣬��������ͼ������ͼ��ռ�
    int *pnInputImg = (int *)malloc(sizeof(int)*(matSrcImage.cols)*(matSrcImage.rows));
    if (NULL == pnInputImg)
    {
        cout << "Could not malloc pnInputImg!" << endl;
    }
    Mat2Array(matBright, pnInputImg);

    // ����ӳ���ռ䣬����ͼ����ǿ�㷨������ͼ��ӳ���
    int nBinNum = 256;
    int *pMap = (int *)malloc(sizeof(int) * nBinNum);
    double *pdNewHistOut = (double *)malloc(sizeof(double) * nBinNum);

    AMHE_main_run(pnInputImg, matSrcImage.cols, matSrcImage.rows, dContrast, nBinNum, pdNewHistOut, pMap);

    // Array--->Mat������ӳ���õ����ͼ��Ŀ������
    cv::Mat matOutputImg(matSrcImage.rows, matSrcImage.cols, CV_8UC1);
    for (int i = 0; i < matSrcImage.rows; i++)
    {
        for (int j = 0; j < matSrcImage.cols; j++)
        {
            matOutputImg.at<uchar>(i, j) = (uchar)(pMap[pnInputImg[i * matSrcImage.cols + j]]);
        }
    }
    cv::imwrite("./Resource/matOutputImg.jpeg", matOutputImg);

    // ��ͨ���ںϳɲ�ɫͼ��
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

    // ��ʾ���ͼ��
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


// ������Ҫ�޸ģ�
// 1��int *pArray
// 2��int *pbyImgeRgb = pArray;
void MainWindow::Mat2Array(cv::Mat matImage, int *pArray)
{
    if (matImage.empty() || pArray == NULL)
    {
        return;
    }

    // û��������أ�������Ϊ1������ΪH*W���Ӷ�ȥ������ѭ��	
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
        // ����ͼ��������CV_32F
        if (CV_32F == matImage.type())
        {
            float *data = matImage.ptr<float>(i);

            for (int j = 0; j < nc; j++)
            {
                *pbyImgeRgb = (uchar)data[j];
                pbyImgeRgb++;
            }
        }
        // ����ͼ��������CV_8U
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






