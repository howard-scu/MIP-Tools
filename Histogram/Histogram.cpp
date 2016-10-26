/*
	Copyright (c) 2016, Lu Xiaohua
	All rights reserved.
*/

#include "itkImage.h"
#include "itkImageToHistogramFilter.h"
#include "itkImageRandomIteratorWithIndex.h"
#include "itkImageFileReader.h"
#include <string>

#include "cmdline.h"


void histogram(const char* filename, int bins)
{
	typedef itk::Image<unsigned short, 3> ImageType;

	const unsigned int MeasurementVectorSize = 1;	 // Grayscale
	const unsigned int binsPerDimension = bins;

	typedef  itk::ImageFileReader< ImageType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(filename);
	reader->Update();

	typedef itk::Statistics::ImageToHistogramFilter< ImageType > ImageToHistogramFilterType;

	ImageToHistogramFilterType::HistogramType::SizeType size(MeasurementVectorSize);
	size.Fill(binsPerDimension);

	ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
	imageToHistogramFilter->SetInput(reader->GetOutput());

	// 默认为true，需要自行设置为false,才能使用自定义上下限
	// ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
	// lowerBound.Fill(0);
	// ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
	// upperBound.Fill(12);
	// imageToHistogramFilter->SetAutoMinimumMaximum(false);
	// imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
	// imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
	imageToHistogramFilter->SetHistogramSize(size);		//每个通道中bin数量
	imageToHistogramFilter->Update();

	ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();

	int top_count = 0;
	char buf[255];

	for (unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
	{
		if (
			((i == 0) && (histogram->GetFrequency(i) > histogram->GetFrequency(i + 1)))
			|| ((i == histogram->GetSize()[0] - 1) && (histogram->GetFrequency(i) > histogram->GetFrequency(i - 1)))
			|| ((histogram->GetFrequency(i) > histogram->GetFrequency(i - 1)) && (histogram->GetFrequency(i) > histogram->GetFrequency(i + 1)))
			)
		{
			top_count++;
			sprintf(buf,"[%7.2f, %7.2f] : %6d\t\t<== top %d", histogram->GetBinMin(0, i), histogram->GetBinMax(0, i),
				histogram->GetFrequency(i), top_count);
			std::cout << buf << std::endl;
		}
		else
		{
			sprintf(buf, "[%7.2f, %7.2f] : %6d", histogram->GetBinMin(0, i), histogram->GetBinMax(0, i),
				histogram->GetFrequency(i));
			std::cout << buf << std::endl;
		}
	}
}

void main(int argc, char* argv[])
{
	cmdline::parser cmd;
	cmd.add<std::string>("filename", 'f', "input filename", true);
	cmd.add<int>("bins", 'n', "histogram bin number", true);
	cmd.parse_check(argc, argv);

	histogram(cmd.get<std::string>("filename").c_str(), cmd.get<int>("bins"));
}
