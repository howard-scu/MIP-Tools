// TpsRegistration.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "TpsRegistration.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkThinPlateSplineKernelTransform.h"
#include "itkPointSet.h"
#include <itkImageFileWriter.h>
#include <math.h>
#include <algorithm>
#include <utility>

bool PtCmp(const Point2D &a, const Point2D &b)
{
	if (a.x < b.x)
		return true;
	else if (a.x == b.x)
		return a.y < b.y;
	else return false;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 TpsRegistration.h
TpsRegistration::TpsRegistration()
{
	return;
}

void TpsRegistration::AddFixContour(std::vector<ImageType::IndexType> &point_list)
{
	m_FixVec.push_back(point_list);
}

void TpsRegistration::AddMovContour(std::vector<ImageType::IndexType> &point_list)
{
	m_MovVec.push_back(point_list);
}

void TpsRegistration::SetFixImage(ImageType::Pointer image)
{
	m_FixedImage = image;
	m_FixedImage->Update();
}

void TpsRegistration::SetMovImage(ImageType::Pointer image)
{
	m_MovingImage = image;
	m_MovingImage->Update();
}

void TpsRegistration::SetFixLandmarks(std::vector<ImageType::IndexType> &point_list)
{
	m_FixedLands = point_list;
	//std::cout << m_FixedLands.size() << std::endl;
	//for(int i=0;i<10;i++)
	//	std::cout << m_FixedLands[i] << std::endl;
}

void TpsRegistration::SetMovLandmarks(std::vector<ImageType::IndexType> &point_list)
{
	m_MovingLands = point_list;
	//std::cout << m_MovingLands.size() << std::endl;
	//for (int i = 0; i<10; i++)
	//	std::cout << m_MovingLands[i] << std::endl;
}

ImageType::Pointer TpsRegistration::GetResult()
{
	return m_ResultImage;
}

ImageType::Pointer TpsRegistration::Threshold(ImageType::Pointer image)
{
	typedef itk::OtsuThresholdImageFilter <ImageType, ImageType>
		FilterType;
	FilterType::Pointer otsuFilter
		= FilterType::New();
	otsuFilter->SetInput(image);
	otsuFilter->SetInsideValue(0);
	otsuFilter->SetOutsideValue(1);
	otsuFilter->Update();
	return otsuFilter->GetOutput();
}

Point2D TpsRegistration::CalcCenter(ImageType::Pointer image)
{
	// 计算二值图像中心
	itk::ImageRegionConstIteratorWithIndex<ImageType> imageIterator(image, image->GetLargestPossibleRegion());

	int mx = 0;
	int my = 0;
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

	return Point2D((int)(mx / sum), (int)(my / sum));
}

Point2D	TpsRegistration::CalcCenter(std::vector<ImageType::IndexType> point_list)
{
	// 计算点队列中心
	unsigned int mx = 0;
	unsigned int my = 0;
	unsigned int sum = 0;

	std::vector<ImageType::IndexType>::iterator it = point_list.begin();

	for (it; it != point_list.end(); it++)
	{
		mx += (*it)[0];
		my += (*it)[1];
		sum++;
	}

	return Point2D((int)(mx / sum), (int)(my / sum));
}

std::vector<std::pair< Point2D, Point2D> > TpsRegistration::FindPairs(ImageType::Pointer image1, Point2DPtr center1,
	ImageType::Pointer image2, Point2DPtr center2)
{
	std::pair<Point2D, Point2D> pointPair(Point2D(center1->x, center1->y),
		Point2D(center2->x, center2->y));
	std::vector<std::pair< Point2D, Point2D> > pointPairs;
	pointPairs.push_back(pointPair);

	int mx = image1->GetLargestPossibleRegion().GetSize()[0];
	int my = image1->GetLargestPossibleRegion().GetSize()[1];

	for (int i = center1->y; i < my; i++)
	{
		ImageType::IndexType idx;
		idx[0] = center1->x;
		idx[1] = i;
		int val = image1->GetPixel(idx);
		if (val == 0)
		{
			for (int j = center2->y; j < my; j++)
			{
				ImageType::IndexType idx1;
				idx1[0] = center2->x;
				idx1[1] = j;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(center1->x, i),
						Point2D(center2->x, j));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->y; i >= 0; i--)
	{
		ImageType::IndexType idx;
		idx[0] = center1->x;
		idx[1] = i;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int j = center2->y; j >= 0; j--)
			{
				ImageType::IndexType idx1;
				idx1[0] = center2->x;
				idx1[1] = j;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(center1->x, i),
						Point2D(center2->x, j));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->x; i < mx; i++)
	{
		ImageType::IndexType idx;
		idx[1] = center1->y;
		idx[0] = i;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int j = center2->x; j < mx; j++)
			{
				ImageType::IndexType idx1;
				idx1[1] = center2->y;
				idx1[0] = j;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(i, center1->y),
						Point2D(j, center2->y));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->x; i >= 0; i--)
	{
		ImageType::IndexType idx;
		idx[1] = center1->y;
		idx[0] = i;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int j = center2->x; j >= 0; j--)
			{
				ImageType::IndexType idx1;
				idx1[1] = center2->y;
				idx1[0] = j;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(i, center1->y),
						Point2D(j, center2->y));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->x, j = center1->y; i < mx && j < mx; i++, j++)
	{
		ImageType::IndexType idx;
		idx[0] = i;
		idx[1] = j;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int p = center2->x, q = center2->y; q < mx && p < mx; p++, q++)
			{
				ImageType::IndexType idx1;
				idx1[0] = p;
				idx1[1] = q;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(i, j),
						Point2D(p, q));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->x, j = center1->y; i < mx && j >= 0; i++, j--)
	{
		ImageType::IndexType idx;
		idx[0] = i;
		idx[1] = j;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int p = center2->x, q = center2->y; p < mx && q >= 0; p++, q--)
			{
				ImageType::IndexType idx1;
				idx1[0] = p;
				idx1[1] = q;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(i, j),
						Point2D(p, q));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->x, j = center1->y; i >= 0 && j >= 0; i--, j--)
	{
		ImageType::IndexType idx;
		idx[0] = i;
		idx[1] = j;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int p = center2->x, q = center2->y; q >= 0 && p >= 0; p--, q--)
			{
				ImageType::IndexType idx1;
				idx1[0] = p;
				idx1[1] = q;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(i, j),
						Point2D(p, q));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	for (int i = center1->x, j = center1->y; i >= 0 && j < mx; i--, j++)
	{
		ImageType::IndexType idx;
		idx[0] = i;
		idx[1] = j;
		int val = image1->GetPixel(idx);

		if (val == 0)
		{
			for (int p = center2->x, q = center2->y; q >= 0 && p >= 0; p--, q++)
			{
				ImageType::IndexType idx1;
				idx1[0] = p;
				idx1[1] = q;
				int val1 = image2->GetPixel(idx1);

				if (val1 == 0)
				{
					std::pair<Point2D, Point2D> pointPair(Point2D(i, j),
						Point2D(p, q));
					pointPairs.push_back(pointPair);
					break;
				}
			}
			break;
		}
	}

	return pointPairs;
}

std::vector<std::pair< Point2D, Point2D> > TpsRegistration::FindPairs(std::vector<std::vector<ImageType::IndexType> >& fixVec,
	std::vector<std::vector<ImageType::IndexType> >& movVec)
{
	std::vector<std::pair< Point2D, Point2D> > pointPairs;
	std::vector<std::vector<ImageType::IndexType> >::iterator it1 = fixVec.begin();
	std::vector<std::vector<ImageType::IndexType> >::iterator it2 = movVec.begin();

	for (it1, it2; it1 != fixVec.end() && it2 != movVec.end(); it1++, it2++)
	{
		Point2D pt1 = CalcCenter(*it1);
		Point2D pt2 = CalcCenter(*it2);
		std::pair<Point2D, Point2D> pointPair(pt1,pt2);
		pointPairs.push_back(pointPair);
	}
	return pointPairs;
}


std::vector<std::pair< Point2D, Point2D> >	TpsRegistration::FindPairs(std::vector<ImageType::IndexType> point_list1, Point2DPtr center1,
	std::vector<ImageType::IndexType> point_list2, Point2DPtr center2)
{
	std::pair<Point2D, Point2D> pointPair(Point2D(center1->x, center1->y),
		Point2D(center2->x, center2->y));
	std::vector<std::pair< Point2D, Point2D> > pointPairs;
	pointPairs.push_back(pointPair);

	std::vector<ImageType::IndexType>::iterator it1 = point_list1.begin();
	std::vector<ImageType::IndexType>::iterator it2 = point_list2.begin();

	std::vector<Point2D> vec_pts1;
	std::vector<Point2D> vec_pts2;
	vec_pts1.resize(8);
	vec_pts2.resize(8);
	for (it1; it1 != point_list1.end(); it1++)
	{
		if (((*it1)[0] == center1->x) && ((*it1)[1] > center1->y))
		{
			vec_pts1[0] = Point2D((*it1)[0], (*it1)[1]);
		}
		else if (((*it1)[0] == center1->x) && ((*it1)[1] < center1->y))
		{
			vec_pts1[1] = Point2D((*it1)[0], (*it1)[1]);
		}
		else if (((*it1)[1] == center1->y) && ((*it1)[0] > center1->x))
		{
			vec_pts1[2] = Point2D((*it1)[0], (*it1)[1]);
		}
		else if (((*it1)[1] == center1->y) && ((*it1)[0] < center1->x))
		{
			vec_pts1[3] = Point2D((*it1)[0], (*it1)[1]);
		}
		else if (abs((*it1)[0] - center1->x) == abs((*it1)[1] - center1->y))
		{
			if ((((*it1)[0] - center1->x) > 0) && (((*it1)[1] - center1->y) > 0))
			{
				vec_pts1[4] = Point2D((*it1)[0], (*it1)[1]);
			}
			if ((((*it1)[0] - center1->x) < 0) && (((*it1)[1] - center1->y) > 0))
			{
				vec_pts1[5] = Point2D((*it1)[0], (*it1)[1]);
			}
			if ((((*it1)[0] - center1->x) < 0) && (((*it1)[1] - center1->y)< 0))
			{
				vec_pts1[6] = Point2D((*it1)[0], (*it1)[1]);
			}
			if ((((*it1)[0] - center1->x) > 0) && (((*it1)[1] - center1->y)< 0))
			{
				vec_pts1[7] = Point2D((*it1)[0], (*it1)[1]);
			}
		}
	}
	for (it2; it2 != point_list2.end(); it2++)
	{
		if (((*it2)[0] == center2->x) && ((*it2)[1] > center2->y))
		{
			vec_pts2[0] = Point2D((*it2)[0], (*it2)[1]);
		}
		else if (((*it2)[0] == center2->x) && ((*it2)[1] < center2->y))
		{
			vec_pts2[1] = Point2D((*it2)[0], (*it2)[1]);
		}
		else if (((*it2)[1] == center2->y) && ((*it2)[0] > center2->x))
		{
			vec_pts2[2] = Point2D((*it2)[0], (*it2)[1]);
		}
		else if (((*it2)[1] == center2->y) && ((*it2)[0] < center2->x))
		{
			vec_pts2[3] = Point2D((*it2)[0], (*it2)[1]);
		}
		else if ((((*it2)[0] - center2->x) == ((*it2)[1] - center2->y))
			&& (((*it2)[0] - center2->x) > 0) && (((*it2)[1] - center2->y) > 0))
		{
			vec_pts2[4] = Point2D((*it2)[0], (*it2)[1]);
		}

		else if ((-((*it2)[0] - center2->x) == ((*it2)[1] - center2->y)) 
			&& (((*it2)[0] - center2->x) < 0) && (((*it2)[1] - center2->y) > 0))
		{
			vec_pts2[5] = Point2D((*it2)[0], (*it2)[1]);
		}
		else if ((-((*it2)[0] - center2->x) == -((*it2)[1] - center2->y)) 
			&& (((*it2)[0] - center2->x) < 0) && (((*it2)[1] - center2->y) < 0))
		{
			vec_pts2[6] = Point2D((*it2)[0], (*it2)[1]);
		}
		else if ((((*it2)[0] - center2->x) == -((*it2)[1] - center2->y)) 
			&& (((*it2)[0] - center2->x) > 0) && (((*it2)[1] - center2->y) < 0))
		{
			vec_pts2[7] = Point2D((*it2)[0], (*it2)[1]);
		}
	}
	
	std::vector<Point2D>::iterator vit1 = vec_pts1.begin();
	std::vector<Point2D>::iterator vit2 = vec_pts2.begin();

	for (vit1, vit2; vit1 != vec_pts1.end() && vit2 != vec_pts2.end(); vit1++, vit2++)
	{
		std::pair<Point2D, Point2D> pointPair(*vit1,*vit2);
		pointPairs.push_back(pointPair);
	}

	return pointPairs;
}

void TpsRegistration::Update()
{
	//ImageType::Pointer image1 = Threshold(m_FixedImage);
	//ImageType::Pointer image2 = Threshold(m_MovingImage);

	//Point2D pt1 = CalcCenter(image1);
	//Point2D pt2 = CalcCenter(image2);

	////std::cout << pt1.x << "\t" << pt1.y << std::endl;
	////std::cout << pt2.x << "\t" << pt2.y << std::endl;

	//std::vector<std::pair< Point2D, Point2D> >  pairs = FindPairs(image1, &pt1, image2, &pt2);
	//TpsReg(image2, pairs);

	//Point2D pt1 = CalcCenter(m_FixedLands);
	//Point2D pt2 = CalcCenter(m_MovingLands);
	//std::cout << pt1.x << "\t" << pt1.y << std::endl;
	//std::cout << pt2.x << "\t" << pt2.y << std::endl;
	//std::vector<std::pair< Point2D, Point2D> >  pairs = FindPairs(m_FixedLands, &pt1, m_MovingLands, &pt2);
	//
	//auto it = pairs.begin();
	//for (it; it != pairs.end(); it++)
	//{
	//	std::cout << "["<<it->first.x << "," << it->first.y << "]\t["
	//		<< it->second.x << "," << it->second.y <<"]"<< std::endl;
	//}
	//TpsReg(m_MovingImage, pairs);
	std::vector<std::pair< Point2D, Point2D> >  pairs = FindPairs(m_FixVec, m_MovVec);
	TpsReg(m_MovingImage, pairs);
}


void TpsRegistration::TpsReg(ImageType::Pointer image, std::vector<std::pair< Point2D, Point2D> >& pairs)
{
	typedef   itk::ImageFileWriter< ImageType >           DeformedImageWriterType;
	typedef   double                                           CoordinateRepType;
	typedef   itk::ThinPlateSplineKernelTransform< CoordinateRepType, 2>   TransformType;
	typedef   itk::Point< CoordinateRepType, 2 >				   PointType;
	typedef   TransformType::PointSetType                      PointSetType;
	typedef   PointSetType::PointIdentifier                    PointIdType;
	typedef   itk::ResampleImageFilter< ImageType, ImageType  >      ResamplerType;
	typedef   itk::LinearInterpolateImageFunction< ImageType, double >              InterpolatorType;

	PointSetType::Pointer sourceLandMarks = PointSetType::New();
	PointSetType::Pointer targetLandMarks = PointSetType::New();
	PointType p1;
	PointType p2;
	PointSetType::PointsContainer::Pointer sourceLandMarkContainer =
		sourceLandMarks->GetPoints();
	PointSetType::PointsContainer::Pointer targetLandMarkContainer =
		targetLandMarks->GetPoints();
	PointIdType id = itk::NumericTraits< PointIdType >::ZeroValue();

	std::vector<std::pair< Point2D, Point2D>>::iterator iter = pairs.begin();
	for (iter; iter != pairs.end(); iter++)
	{
		p1[0] = iter->first.x;
		p1[1] = iter->first.y;
		p2[0] = iter->second.x;
		p2[1] = iter->second.y;
		sourceLandMarkContainer->InsertElement(iter - pairs.begin(), p1);
		targetLandMarkContainer->InsertElement(iter - pairs.begin(), p2);
	}

	TransformType::Pointer tps = TransformType::New();
	tps->SetSourceLandmarks(sourceLandMarks);
	tps->SetTargetLandmarks(targetLandMarks);
	tps->ComputeWMatrix();

	ResamplerType::Pointer resampler = ResamplerType::New();
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	resampler->SetInterpolator(interpolator);
	ImageType::SpacingType spacing = image->GetSpacing();
	ImageType::PointType   origin = image->GetOrigin();
	ImageType::DirectionType direction = image->GetDirection();
	ImageType::RegionType region = image->GetBufferedRegion();
	ImageType::SizeType   size = region.GetSize();

	resampler->SetOutputSpacing(spacing);
	resampler->SetOutputDirection(direction);
	resampler->SetOutputOrigin(origin);
	resampler->SetSize(size);
	resampler->SetTransform(tps);
	resampler->SetOutputStartIndex(region.GetIndex());
	resampler->SetInput(image);
	resampler->Update();

	m_ResultImage = resampler->GetOutput();
	//DeformedImageWriterType::Pointer deformedImageWriter =
	//	DeformedImageWriterType::New();
	//deformedImageWriter->SetInput(resampler->GetOutput());
	//deformedImageWriter->SetFileName("t2adsf.mha");
	//try
	//{
	//	deformedImageWriter->Update();
	//}
	//catch (itk::ExceptionObject & excp)
	//{
	//	std::cerr << "Exception thrown " << std::endl;
	//	std::cerr << excp << std::endl;
	//}
}
