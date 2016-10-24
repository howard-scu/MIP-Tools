/*
	Copyright (c) 2016, Lu Xiaohua
	All rights reserved.
*/

#ifndef IMAGE_PROCESSING_FUNC_H
#define IMAGE_PROCESSING_FUNC_H

#include "itkConnectedThresholdImageFilter.h"
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <vector>


// ���ܣ�	���������ָ�
// ���룺	@input_fn		����ͼ���ļ���
//			@output_fn		���ͼ���ļ���
//			@index			���ӵ�
//			@lower_thres	��ֵ����
//			@upper_thres	��ֵ����
// �����	�Ƿ�ɷָ�

bool DoSegmentation(const char* input_fn, const char* output_fn,
					std::vector<std::vector<int>> &index, int lower_thres, int upper_thres)
{
	if (!input_fn || !input_fn) return false;

	typedef   float           InternalPixelType;
	const     unsigned int    Dimension = 3;
	typedef itk::Image< InternalPixelType, Dimension >  InternalImageType;


	typedef unsigned char                            OutputPixelType;
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
	typedef itk::CastImageFilter< InternalImageType, OutputImageType >
		CastingFilterType;
	CastingFilterType::Pointer caster = CastingFilterType::New();

	typedef  itk::ImageFileReader< InternalImageType > ReaderType;
	typedef  itk::ImageFileWriter<  OutputImageType  > WriterType;

	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	reader->SetFileName(input_fn);
	writer->SetFileName(output_fn);

	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
		return false;
	}

	typedef itk::ConnectedThresholdImageFilter< InternalImageType,
		InternalImageType > ConnectedFilterType;

	ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();
	connectedThreshold->SetInput(reader->GetOutput());
	caster->SetInput(connectedThreshold->GetOutput());
	writer->SetInput(caster->GetOutput());

	const InternalPixelType lowerThreshold = lower_thres;
	const InternalPixelType upperThreshold = upper_thres;

	connectedThreshold->SetLower(lowerThreshold);
	connectedThreshold->SetUpper(upperThreshold);
	connectedThreshold->SetReplaceValue(255);

	auto it	 =	index.begin();
	auto end =	index.end();
	
	for (it; it != end; it++)
	{
		InternalImageType::IndexType  _index;
		_index[0] = (*it)[0];
		_index[1] = (*it)[1];
		_index[2] = (*it)[2];
		connectedThreshold->AddSeed(_index);
	}

	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
		return false;
	}
	return true;
}
#endif  // IMAGE_PROCESSING_FUNC_H
