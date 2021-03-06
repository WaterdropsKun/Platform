#pragma once

#include <opencv2/core.hpp>

#include <string>

#include "c_transform.h"

// BarrelCorrection_CPU
#include "CPUBCCommon.h"
#include "CPUCommon.h"
#include "CPUBC.h"
#include "CPUBarrelCorrection.h"

#include "ISPVideoAlg.h"

// DebugMK
#include "debug_log.h"

class CBarrelCorrection
{
public:
    CBarrelCorrection();
    ~CBarrelCorrection();

private:
    // BarrelCorrection
    float m_fK0;
    float m_fK1;

    int m_nBarrelCorrectionFixedSizeFlag;    

    // VirtualStitch
    int m_nVirtualStitchFlag;
    int m_nZoom;
    int m_nBarrelUp;
    int m_nUpDownCurveRatio;
    int m_nUpDownRatio;   

    int m_nFinalWidth;
    int m_nFinalHeight;
    

public:
    void SetK0K1(float K0, float K1);

    float GetK0();
    float GetK1();

    cv::Mat BarrelCorrection(cv::Mat matSrcImage);
    cv::Mat VirtualStitch(cv::Mat matSrcImage, float fK[]);

    // CPU�������YUV
    cv::Mat BarrelCorrection_CPU(cv::Mat& matSrcImage);

    void imreadNV12(unsigned char* pu8Y, unsigned char* pu8UV, int nWidth, int nHeight, char *pchFilePath);
    cv::Mat imshowNV12(unsigned char* pu8Y, unsigned char* pu8UV, int nWidth, int nHeight);
    void imwriteNV12(unsigned char* pu8Y, unsigned char* pu8UV, int nWidth, int nHeight, char *pchFilePath);
    void Mat2NV12(cv::Mat matSrcImage, unsigned char *pu8Y, unsigned char *pu8UV, int nWidth, int nHeight);


private:
    CTransform m_cTransform;

    int m_nBaseK0;
    int m_nBaseK1;

    cv::Mat m_matBarrelCorrectionImage;
    int m_nBarrelCorrectionWidth;
    int m_nBarrelCorrectionHeight;

    cv::Mat m_matVirtualStitchImage;
    int m_nVirtualStitchWidth;
    int m_nVirtualStitchHeight;

};

