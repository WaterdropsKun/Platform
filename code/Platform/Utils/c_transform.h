#pragma once
#include <opencv2/core.hpp>

typedef unsigned char BYTE;

class CTransform
{
public:
	CTransform();
    ~CTransform();

	//���б���
	//���к���
    int Mat2Array(cv::Mat matImage, BYTE *pArray);
	cv::Mat Array2Mat(BYTE* pbyImageRgb, int nHeight, int nWeight, int nChannels);
	//˽�к���
	//˽�б���
	cv::Mat c_matImage;

	//���������ͱ���
};

