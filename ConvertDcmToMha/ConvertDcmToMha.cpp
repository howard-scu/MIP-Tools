/*
  Copyright (c) 2016, Lu Xiaohua
  All rights reserved.
*/

#ifndef CONVERT_DCM_TO_MHA_CPP
#define CONVERT_DCM_TO_MHA_CPP

#include "cmdline.h"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageFileWriter.h>


typedef itk::Image< unsigned short, 3 >		ImageType;

void ConvertDcmToMha(const char* path, const char* filename)
{
	// use vtk to read dicom
    vtkSmartPointer<vtkDICOMImageReader> reader =
		vtkSmartPointer<vtkDICOMImageReader>::New();
	reader->SetDirectoryName(path);
	reader->Update();

	// convert vtkimagedata to itkimage
	typedef itk::VTKImageToImageFilter<ImageType>		VTK2ITK;

	VTK2ITK::Pointer v2ifilter = VTK2ITK::New();
	v2ifilter->SetInput(reader->GetOutput());
	v2ifilter->Update();

	// output meta image
	typedef  itk::ImageFileWriter<ImageType>			WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName(filename);
	writer->SetInput(v2ifilter->GetOutput());
	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
	}
}


int main(int argc, char* argv[])
{
	cmdline::parser a;
	a.add<string>("path", 'p', "dicom path");
	a.add<string>("filename", 'f', "meta file name");
	a.parse_check(argc, argv);

	ConvertDcmToMha(a.get<string>("path").c_str(),	/* dicom dataset directory */
		a.get<string>("filename").c_str());			/* mha file name(.mha)*/
}

#endif  // CONVERT_DCM_TO_MHA_CPP
