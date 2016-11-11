#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkBinaryThresholdImageFilter.h"

using namespace std;

typedef unsigned char  PixelType;
typedef itk::Image< PixelType, 2 >  ImageType;

void PrintImage(ImageType::Pointer image)
{
	int nx = image->GetLargestPossibleRegion().GetSize()[0];
	int ny = image->GetLargestPossibleRegion().GetSize()[1];
	ImageType::IndexType idx;
	for (int i = 0; i < nx; i++)
	{
		for (int j = 0; j < ny; j++)
		{
			idx[0] = i;
			idx[1] = j;

			int value = image->GetPixel(idx);
			std::cout << value << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char *argv[])
{
	// 读取图像
	typedef  itk::ImageFileReader< ImageType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("test.bmp");
	reader->Update();

	// 二值化
	typedef itk::BinaryThresholdImageFilter <ImageType, ImageType>
		BinaryThresholdImageFilterType;

	BinaryThresholdImageFilterType::Pointer thresholdFilter
		= BinaryThresholdImageFilterType::New();
	thresholdFilter->SetInput(reader->GetOutput());
	thresholdFilter->SetLowerThreshold(1);
	thresholdFilter->SetUpperThreshold(255);
	thresholdFilter->SetInsideValue(1);
	thresholdFilter->SetOutsideValue(0);
	thresholdFilter->Update();

	// 输出图像
	//PrintImage(thresholdFilter->GetOutput());

	// 初始化输出图像
	ImageType::Pointer maskImage = ImageType::New();
	maskImage->SetRegions(reader->GetOutput()->GetLargestPossibleRegion());
	maskImage->Allocate();

	itk::ImageRegionIterator<ImageType> it(maskImage, maskImage->GetLargestPossibleRegion());
	while (!it.IsAtEnd())
	{
		it.Set(0);
		++it;
	}

	// 查找起始点
	ImageType::Pointer originImage = thresholdFilter->GetOutput();
	int nx = originImage->GetLargestPossibleRegion().GetSize()[0];
	int ny = originImage->GetLargestPossibleRegion().GetSize()[1];

	ImageType::IndexType start_idx;		// 起始节点 

	bool find_start_pt = false;
	for (int i = 0; i < nx && !find_start_pt; i++)
	{
		for (int j = 0; j < ny && !find_start_pt; j++)
		{
			start_idx[0] = i;
			start_idx[1] = j;
			if (originImage->GetPixel(start_idx) == 1)
			{
				// 搜索到起始点
				find_start_pt = true;
			}
		}
	}
	cout << "搜索到起始点:  " << start_idx[0] << " " << start_idx[1] << endl;

	// 迭代搜索相继节点
	bool success = false;
	int  search_direction = 0;
	int  directions[8][2] = { { 0,1 },{ 1,1 },{ 1,0 },{ 1,-1 },{ 0,-1 },{ -1,-1 },{ -1,0 },{ -1,1 } };

	ImageType::IndexType cur_idx;		// 当前节点
	ImageType::IndexType nxt_idx;		// 后续搜索节点
	cur_idx[0] = start_idx[0];
	cur_idx[1] = start_idx[1];
	bool is_tail = true;				// 尾节点需要搜索全部
	maskImage->SetPixel(start_idx, 1);
	int t = 255;
	while (!success)
	{
		bool is_find_next = false;

		if (is_tail)
		{
			//搜索8位置
			for (int i = 0; i < 8; i++)
			{
				nxt_idx[0] = cur_idx[0] + directions[i][0];
				nxt_idx[1] = cur_idx[1] + directions[i][1];

				if (originImage->GetPixel(nxt_idx) == 1) // 是边界点
				{
					// 进行下一轮迭代
					search_direction = i;
					maskImage->SetPixel(nxt_idx, t);

					cur_idx[0] = nxt_idx[0];
					cur_idx[1] = nxt_idx[1];
					is_find_next = true;
					is_tail = false;
					break; // 结束该点查找
				}
			}
			if (!is_find_next)
			{
				is_tail = true;
				success = true;
			}
		}
		else
		{
			//搜索5位置
			for (int i = 0; i < 5; i++)
			{
				int k = (search_direction + i - 2);

				//规范k
				if (k < 0) k += 8;
				else if (k >= 8) k = k % 8;

				nxt_idx[0] = cur_idx[0] + directions[k][0];
				nxt_idx[1] = cur_idx[1] + directions[k][1];

				if (nxt_idx[0] < 0 || nxt_idx[0] > nx || nxt_idx[1] < 0 || nxt_idx[1] >  ny)
				{
					//出界处理：跳过
					continue;
				}

				if (nxt_idx[0] == start_idx[0] && nxt_idx[1] == start_idx[1])
				{
					// 回到起始点 结束
					success = true;
					break;
				}


				if (originImage->GetPixel(nxt_idx) == 1) // 是边界点
				{
					// 进行下一轮迭代
					search_direction = k;
					maskImage->SetPixel(nxt_idx, t);

					cur_idx[0] = nxt_idx[0];
					cur_idx[1] = nxt_idx[1];
					is_find_next = true;
					break; // 结束该点查找
				}
				else if (originImage->GetPixel(nxt_idx) != 1)
				{
					maskImage->SetPixel(nxt_idx, t / 2);
				}

			}

			// 注意此处的处理
			if (!is_find_next)
			{
				// 回头查找
				search_direction = (search_direction - 4);
				is_tail = true;
				//t++;
			}

		}
	}

	typedef  itk::ImageFileWriter<  ImageType  > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName("test.mask.bmp");
	writer->SetInput(maskImage);
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
}

