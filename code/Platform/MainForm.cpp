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
    ui.BC_nK0_le->setText(QString::number(ui.BC_nK0_hs->value()));
    ui.BC_nK1_le->setText(QString::number(ui.BC_nK1_hs->value()));

    int BC_nK0 = ui.BC_nK0_hs->value();
    int BC_nK1 = ui.BC_nK1_hs->value();


    m_cBarrelCorrection.SetK0(BC_nK0);
    m_cBarrelCorrection.SetK1(BC_nK1);

    this->BarrelCorrectionTest();
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
    //this->Test();
}


void MainWindow::Test()
{
    
}


void MainWindow::BarrelCorrectionTest()
{
#if 1
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
#endif
    //cv::Mat matSrcImage = cv::imread("./Resource/2018122409130887.jpeg");
    if (matSrcImage.empty())
    {
        cout << "Could not load matSrcImage!" << endl;
    }


#if 0 // RGB����У��
    cv::Mat matSrcImageShow;   
    cv::resize(matSrcImage, matSrcImageShow, cv::Size(matSrcImage.cols / 4, matSrcImage.rows / 4));
    cv::imshow("matSrcImageShow", matSrcImageShow);
    
    cv::Mat matBarrelCorrectionImage = m_cBarrelCorrection.BarrelCorrection(matSrcImage);
    cv::resize(matBarrelCorrectionImage, matBarrelCorrectionImage, cv::Size(matBarrelCorrectionImage.cols / 4, matBarrelCorrectionImage.rows / 4));
    imshow("BarrelCorrectionImage", matBarrelCorrectionImage);
#endif

#if 1 // YUV����У��
    int SrcHeight = 1536;
    int SrcWidth = 2048;

    cv::Mat matSrcImageShow;
    cv::resize(matSrcImage, matSrcImageShow, cv::Size(SrcWidth, SrcHeight));
    cv::imshow("matSrcImageShow", matSrcImageShow);

    this->m_cBarrelCorrection.BarrelCorrection_CPU_2("");

    cv::Mat rgbImgResult;
    rgbImgResult = this->m_cBarrelCorrection.BarrelCorrection_CPU(matSrcImageShow);
    cv::imshow("rgbimg", rgbImgResult);
#endif

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






