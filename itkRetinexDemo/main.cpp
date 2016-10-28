/*
	Copyright (c) 2016, Lu Xiaohua
	All rights reserved.
*/

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
#include <itkLogImageFilter.h>
#include <itkLog10ImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkPowImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkShiftScaleImageFilter.h>
#include <iostream>


int main(int argc, char*argv[])
{
	// Log(Rc(x,y)) = Log(Sc(x,y)) − Log(Sc(x,y)*G(x,y))
	typedef  unsigned short   InputPixelType;
	typedef  float			  InternalPixelType;
	typedef itk::Image< InputPixelType, 3 >   		ImageType;
	typedef itk::Image< InternalPixelType, 3 >   	InternalImageType;

	typedef itk::ImageFileReader< ImageType >  			ReaderType;
	typedef itk::ImageFileWriter< InternalImageType >  	WriterType;

	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(INPUT_FILENAME);
	reader->Update();			// reader->GetOutput() is Sc(x,y)

	typedef itk::CastImageFilter< ImageType, InternalImageType >   		 CastImageType; 
	typedef itk::ShiftScaleImageFilter< InternalImageType, ImageType >   ReCastImageType;

	CastImageType::Pointer caster = CastImageType::New();
	caster->SetInput(reader->GetOutput());
	caster->Update();

	typedef itk::Log10ImageFilter< InternalImageType, InternalImageType >   LogImageType;
	LogImageType::Pointer logImage = LogImageType::New();		// Log(Sc(x,y))
	logImage->SetInput(caster->GetOutput());		

	typedef itk::DiscreteGaussianImageFilter< InternalImageType, InternalImageType >  GaussianImageType;
	GaussianImageType::Pointer gaussianFilter = GaussianImageType::New();		// Sc(x,y)*G(x,y)
	gaussianFilter->SetInput(caster->GetOutput());
	gaussianFilter->SetVariance(9);
	gaussianFilter->SetMaximumKernelWidth(7);

	LogImageType::Pointer logImage1 = LogImageType::New();
	logImage1->SetInput(gaussianFilter->GetOutput());			// Log(Sc(x,y)*G(x,y))

	typedef itk::SubtractImageFilter< InternalImageType, InternalImageType, InternalImageType >  SubtractImageType;
	SubtractImageType::Pointer subImageFilter = SubtractImageType::New();		// Log(Rc(x,y))
	subImageFilter->SetInput1(logImage->GetOutput());
	subImageFilter->SetInput2(logImage1->GetOutput());
	subImageFilter->Update();

	typedef itk::ImageDuplicator< InternalImageType > DuplicatorType;
	DuplicatorType::Pointer duplicator = DuplicatorType::New();
	duplicator->SetInputImage(subImageFilter->GetOutput());
	duplicator->Update();

	itk::ImageRegionIterator<InternalImageType> imageIterator(duplicator->GetOutput(),
		duplicator->GetOutput()->GetLargestPossibleRegion());

	while (!imageIterator.IsAtEnd())
	{
		imageIterator.Set(10.0);
		++imageIterator;
	}


	typedef itk::PowImageFilter< InternalImageType, InternalImageType, InternalImageType >  PowImageFilter;
	PowImageFilter::Pointer powImageFilter = PowImageFilter::New();			// power exponentiation to inverse log
	powImageFilter->SetInput2(subImageFilter->GetOutput());
	powImageFilter->SetInput1(duplicator->GetOutput());


	ReCastImageType::Pointer rcaster = ReCastImageType::New();
	rcaster->SetInput(powImageFilter->GetOutput());
	rcaster->SetScale(1000);
	rcaster->Update();

	WriterType::Pointer writer = WriterType::New();
	writer->SetInput(powImageFilter->GetOutput());
	writer->SetFileName(INPUT_FILENAME);
	writer->Update();
 	return EXIT_SUCCESS;
}
