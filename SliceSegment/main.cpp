#include "itkConnectedThresholdImageFilter.h"
#include "itkRegionGrowingImageFilter.h"
#include "itkImage.h"
#include "itkImageRegion.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkExtractImageFilter.h"
#include <itkJoinSeriesImageFilter.h>
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"


//#define IMG3D //segment image in 3d
//#define W2D	//write slice segment result
#define W3D		//write segment image in 3d
#define DRAW_PT	// draw seed point
//#define CONN_COM // largest connected component

#define m_lower 850		//1.1240//2.700
#define m_upper	1600	//2400//1500
#define seed_x 82
#define seed_y 104


int main(int argc, char *argv[])
{

	typedef   unsigned short  PixelType;
	typedef itk::Image< PixelType, 3 >  Image3DType;
	typedef itk::Image< PixelType, 2 >  Image2DType;

	typedef  itk::ImageFileReader< Image3DType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("out4.mha");
	reader->Update();

#ifdef  IMG3D
	typedef itk::ImageFileWriter<  Image3DType  > WriterType;
	typedef itk::RegionGrowingImageFilter< Image3DType,
		Image3DType > ConnectedFilterType;
	ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();
	connectedThreshold->SetInput(reader->GetOutput());
	connectedThreshold->SetLower(730);
	connectedThreshold->SetUpper(1500);
	connectedThreshold->SetReplaceValue(255);

	Image3DType::IndexType  index;
	index[0] = 69;
	index[1] = 90;
	index[2] = 11;
	connectedThreshold->SetSeed(index);

	WriterType::Pointer writer = WriterType::New();
	writer->SetInput(connectedThreshold->GetOutput());
	writer->SetFileName("out2.seg.mha");
	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
	}
#else
	// 获取slice范围[0,slice_max-1]
	int slice_max = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[2];
	
	// 合并初始化
	typedef itk::JoinSeriesImageFilter<Image2DType, Image3DType> JoinSeriesFilterType;
	JoinSeriesFilterType::Pointer joinFilter = JoinSeriesFilterType::New();


	Image2DType::IndexType  index;
	Image3DType::IndexType inx3d;

	index[0] = seed_x;
	index[1] = seed_y;
	// 合并结果
	for (unsigned int slice = 0; slice < slice_max; ++slice)
	{
		inx3d[0] = index[0];
		inx3d[1] = index[1];
		inx3d[2] = slice;

		int val3d = reader->GetOutput()->GetPixel(inx3d);

		std::cout << "===============================\n";
		std::cout << "该轮迭代参数\n";
		std::cout << "index: [" << inx3d[0] << ", "
			<< inx3d[1] << ", "
			<< inx3d[2] << " ]" << std::endl;
		std::cout << "value: " << val3d << std::endl;

		// 提取出每层
		typedef itk::ExtractImageFilter< Image3DType, Image2DType > SliceType;
		SliceType::Pointer sliceFilter = SliceType::New();
		sliceFilter->SetDirectionCollapseToSubmatrix();
		Image3DType::RegionType inputRegion =
			reader->GetOutput()->GetLargestPossibleRegion();
		Image3DType::SizeType size = inputRegion.GetSize();
		size[2] = 0;

		Image3DType::IndexType start = inputRegion.GetIndex();
		//const unsigned int sliceNumber = 0;
		start[2] = slice;

		Image3DType::RegionType desiredRegion;
		desiredRegion.SetSize(size);
		desiredRegion.SetIndex(start);

		sliceFilter->SetExtractionRegion(desiredRegion);
		sliceFilter->SetInput(reader->GetOutput());
		sliceFilter->Update();

		// 对每层进行区域生长
		typedef itk::ConnectedThresholdImageFilter< Image2DType,
			Image2DType > ConnectedFilterType;
		ConnectedFilterType::Pointer connectedThreshold = ConnectedFilterType::New();
		connectedThreshold->SetInput(sliceFilter->GetOutput());
		connectedThreshold->SetLower(m_lower);
		connectedThreshold->SetUpper(m_upper);
		connectedThreshold->SetReplaceValue(1);

		connectedThreshold->SetSeed(index);
		connectedThreshold->Update();


#ifdef CONN_COM
		typedef itk::BinaryBallStructuringElement<
			Image2DType::PixelType, 2>   StructuringElementType;
		StructuringElementType structuringElement;
		structuringElement.SetRadius(1);
		structuringElement.CreateStructuringElement();

		typedef itk::BinaryMorphologicalOpeningImageFilter <Image2DType, Image2DType, StructuringElementType>
			BinaryOpeningImageFilterType;

		BinaryOpeningImageFilterType::Pointer openingFilter
			= BinaryOpeningImageFilterType::New();
		openingFilter->SetKernel(structuringElement);
		openingFilter->SetInput(connectedThreshold->GetOutput());

		typedef itk::ConnectedComponentImageFilter < Image2DType, Image2DType >
			ConnectedComponentImageFilterType;
		typedef itk::LabelShapeKeepNObjectsImageFilter< Image2DType >
			LabelShapeKeepNObjectsImageFilterType; 


		ConnectedComponentImageFilterType::Pointer connectedComponentFilter =
			ConnectedComponentImageFilterType::New();
		connectedComponentFilter->SetInput(openingFilter->GetOutput());			//input from SliceBySliceImageFilter
																				// connectedComponentFilter->Update();			// should not use filter update

		LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
		labelShapeKeepNObjectsImageFilter->SetInput(connectedComponentFilter->GetOutput());
		labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
		labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
		labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
		labelShapeKeepNObjectsImageFilter->Update();		// should not use filter update
		joinFilter->SetInput(slice, labelShapeKeepNObjectsImageFilter->GetOutput());
#else
		joinFilter->SetInput(slice, connectedThreshold->GetOutput());
#endif // CONN_COM


#ifdef DRAW_PT
		Image2DType::IndexType tmp_idx;
		for (int i = index[0] - 1; i <= index[0] + 1; i++)
		{
			for (int j = index[1] - 1; j <= index[1] + 1; j++)
			{
				tmp_idx[0] = i;
				tmp_idx[1] = j;
#ifdef CONN_COM
				connectedThreshold->GetOutput()->SetPixel(tmp_idx, 2);
#else
				joinFilter->SetInput(slice, connectedThreshold->GetOutput());
#endif // CONN_COM
			}
		}
#endif // DRAW_PT


#ifdef W2D

		typedef  itk::ImageFileWriter<  Image2DType  > WriterType;
		WriterType::Pointer writer = WriterType::New();
		char fn[25];
		sprintf(fn,"seg.%d.mha",slice);
		writer->SetFileName(fn);
		writer->SetInput(connectedThreshold->GetOutput());
		try
		{
			writer->Update();
		}
		catch (itk::ExceptionObject & err)
		{
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return EXIT_FAILURE;
		}

#endif // W2D

		if (slice == slice_max-1) break;


#ifdef CONN_COM
		// 计算下轮迭代种子点坐标
		itk::ImageRegionConstIteratorWithIndex<Image2DType> imageIterator(labelShapeKeepNObjectsImageFilter->GetOutput(),
			labelShapeKeepNObjectsImageFilter->GetOutput()->GetLargestPossibleRegion());
#else
		// 计算下轮迭代种子点坐标
		itk::ImageRegionConstIteratorWithIndex<Image2DType> imageIterator(connectedThreshold->GetOutput(),
			connectedThreshold->GetOutput()->GetLargestPossibleRegion());
#endif // CONN_COM
		int mx=0;
		int my=0;
		int sum = 0;

		while (!imageIterator.IsAtEnd())
		{
			if (imageIterator.Get() != 0)
			{
				mx += imageIterator.GetIndex()[0];
				my += imageIterator.GetIndex()[1];
				sum++;
			}
			++imageIterator;
		}
		//std::cout << "mx/my: [" << mx << ", "
		//	<< my << " ]" << std::endl;
		//std::cout << "sum: " << sum << std::endl;

		// 坐标如下
		index[0] = (int)(mx / sum);
		index[1] = (int)(my / sum);

		//bool find = true;
		inx3d[0] = index[0];
		inx3d[1] = index[1];
		inx3d[2] = slice + 1;

		val3d = reader->GetOutput()->GetPixel(inx3d);

		std::cout << "下轮迭代参数\n";
		std::cout << "index: [" << inx3d[0] << ", "
			<< inx3d[1] << ", "
			<< inx3d[2] << " ]" << std::endl;
		std::cout << "value: " << val3d << std::endl;
		std::cout << "===============================\n";

		bool find = true;
		if (val3d < m_lower || val3d > m_upper)
		{
			// 种子点不符合规则
			find = false;
		}
		
		int directions[8][2] = { { 0,-1 },{ -1,-1 },{ -1,0 },{ -1,1 },{ 0,1 },{ 1,1 },{ 1,0 },{ 1,-1 } };
		int step = 1;
		int nd = 0;
		while (find == false)
		{
			inx3d[0] = index[0] + step*directions[nd][0];
			inx3d[1] = index[1] + step*directions[nd][0];

			val3d = reader->GetOutput()->GetPixel(inx3d);
			if (val3d > m_lower && val3d < m_upper)
			{
				index[0] = inx3d[0];
				index[1] = inx3d[1];
				find = true;

				std::cout << "-------------------------------\n";
				std::cout << "搜索迭代参数\n";
				std::cout << "index: [" << inx3d[0] << ", "
					<< inx3d[1] << ", "
					<< inx3d[2] << " ]" << std::endl;
				std::cout << "value: " << val3d << std::endl;
				std::cout << "-------------------------------\n";

				break;
			}
			nd++;
			if (nd % 8 == 0)
			{
				step++;
				nd = 0;
			}

		}
	}

	// z轴的Origin和Spacing
	joinFilter->SetOrigin(reader->GetOutput()->GetOrigin()[2]);
	joinFilter->SetSpacing(reader->GetOutput()->GetSpacing()[2]);
#ifdef W3D
	typedef  itk::ImageFileWriter<  Image3DType  > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName("out4.ss.mha");
	writer->SetInput(joinFilter->GetOutput());
	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & err)
	{
		std::cerr << "ExceptionObject caught !" << std::endl;
		std::cerr << err << std::endl;
		return EXIT_FAILURE;
	}
#endif // DEBUG

#endif //  IMG2D
	return EXIT_SUCCESS;
}


