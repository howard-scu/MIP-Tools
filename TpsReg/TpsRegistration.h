#ifdef TPSREGISTRATION_EXPORTS
#define TPSREGISTRATION_API __declspec(dllexport)
#else
#define TPSREGISTRATION_API __declspec(dllimport)
#endif

#include <itkImage.h>
#include <itkSmartPointer.h>
#include <iostream>

typedef   struct  Point2D
{
	unsigned short x;
	unsigned short y;
	Point2D(unsigned short mx, unsigned short my) :x(mx), y(my) {}
	Point2D(){}
}Point2D;
typedef   Point2D*	Point2DPtr;
typedef   unsigned char  PixelType;
typedef   typename itk::Image< PixelType, 2 > ImageType;

class TPSREGISTRATION_API TpsRegistration 
{
public:
	TpsRegistration(void);
	void SetFixImage(ImageType::Pointer image);
	void SetMovImage(ImageType::Pointer image);
	void AddFixContour(std::vector<ImageType::IndexType> &point_list);
	void AddMovContour(std::vector<ImageType::IndexType> &point_list);
	ImageType::Pointer GetResult();
	void Update();

private:
	ImageType::Pointer		Threshold(ImageType::Pointer image);
	Point2D					CalcCenter(ImageType::Pointer image);
	std::vector<std::pair< Point2D, Point2D> >		FindPairs(ImageType::Pointer image1, Point2DPtr center1,
		ImageType::Pointer image2, Point2DPtr center2);
	std::vector<std::pair< Point2D, Point2D> >		FindPairs(std::vector<ImageType::IndexType> image1, Point2DPtr center1,
		std::vector<ImageType::IndexType> image2, Point2DPtr center2);
	void					TpsReg(ImageType::Pointer image, std::vector<std::pair< Point2D, Point2D> > &pairs);
	Point2D					CalcCenter(std::vector<ImageType::IndexType> point_list);
	void SetFixLandmarks(std::vector<ImageType::IndexType> &point_list);
	void SetMovLandmarks(std::vector<ImageType::IndexType> &point_list);
	ImageType::Pointer	m_FixedImage;
	ImageType::Pointer	m_MovingImage;
	ImageType::Pointer	m_ResultImage;
	std::vector<ImageType::IndexType>	m_FixedLands;
	std::vector<ImageType::IndexType>	m_MovingLands;
};

