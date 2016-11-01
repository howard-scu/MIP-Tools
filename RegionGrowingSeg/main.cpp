#include <itkCastImageFilter.h>                                                                
#include <itkConnectedThresholdImageFilter.h>                                                  
#include <itkImageFileReader.h>                                                                
#include <itkRegionGrowingImageFilter.h>                                                       
#include <sstream>                                                                             
#include <vector>                                                                              
#include <vtkActor.h>                                                                          
#include <vtkActor2D.h>                                                                        
#include <vtkAssemblyPath.h>                                                                   
#include <vtkCell.h>                                                                           
#include <vtkCommand.h>                                                                        
#include <vtkCornerAnnotation.h>                                                               
#include <vtkImageActor.h>                                                                     
#include <vtkImageCast.h>                                                                      
#include <vtkImageData.h>                                                                      
#include <vtkImageViewer2.h>                                                                   
#include <vtkInteractorStyleImage.h>                                                           
#include <vtkMath.h>                                                                           
#include <vtkMetaImageReader.h>                                                                
#include <vtkObjectFactory.h>                                                                  
#include <vtkPointData.h>                                                                      
#include <vtkPropPicker.h>                                                                     
#include <vtkRenderWindow.h>                                                                   
#include <vtkRenderWindowInteractor.h>                                                         
#include <vtkRenderer.h>                                                                       
#include <vtkSmartPointer.h>                                                                   
#include <vtkTextMapper.h>                                                                     
#include <vtkTextProperty.h>                                                                   
#include <vtkVersion.h>                                                                        
#include <vtkSeedWidget.h>
#include <vtkSeedRepresentation.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include "vtkImageInteractionCallback.h"
#include "vtkUserInteractorStyleImage.h"
#include "vtkRegionSelectionWidget.h"
#include <vtkSphereSource.h>


#include "cmdline.h"
#include <Winbase.h>

//--input = out1.g.mha --output = out1.gs.mha --upper = 1100 --lower = 950 --window = 1500 --level = 1000
//usage: LiverSegDemo.exe --input=out1.g.mha --output=out1.gs.mha --upper=1100 --lower=950 --window=1500 --level=1000
void main(int argc, char* argv[])
{
	cmdline::parser cmd;
	cmd.add<string>("input", 'f', "input filename", true);
	cmd.add<string>("output", 'o', "output filename", true);
	cmd.add<int>("upper", 'u', "upper threshold", true);
	cmd.add<int>("lower", 'l', "lower threshold", true);
	cmd.add<int>("window", 'w', "display window width", 1500);
	cmd.add<int>("level", 'c', "display window level", 3000);
	cmd.parse_check(argc, argv);

	// 读取图像
	vtkSmartPointer<vtkMetaImageReader> reader =
		vtkSmartPointer<vtkMetaImageReader>::New();
	reader->SetFileName(cmd.get<string>("input").c_str());
	reader->Update();

	// 可视化
	vtkSmartPointer<vtkImageViewer2> imageViewer =
		vtkSmartPointer<vtkImageViewer2>::New();
	imageViewer->SetInputConnection(reader->GetOutputPort());

	// 拾取器
	vtkSmartPointer<vtkPropPicker> propPicker =
		vtkSmartPointer<vtkPropPicker>::New();
	propPicker->PickFromListOn();

	// 设置拾取对象
	vtkImageActor* imageActor = imageViewer->GetImageActor();
	propPicker->AddPickList(imageActor);

	// 不使用插值计算，获取体素数值
	imageActor->InterpolateOff();

	vtkSmartPointer<vtkImageInteractionCallback> callback =
		vtkSmartPointer<vtkImageInteractionCallback>::New();
	callback->SetViewer(imageViewer);
	callback->SetPicker(propPicker);

	// 设置annotation
	//	---------
	//	|2	   3|
	//	|0	   1|
	//	---------
	vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation =
		vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation->SetLinearFontScaleFactor(2);
	cornerAnnotation->SetNonlinearFontScaleFactor(1);
	cornerAnnotation->SetMaximumFontSize(20);
	cornerAnnotation->SetText(0, "Off Image");
	cornerAnnotation->SetText(3, "<window>\n<level>");
	cornerAnnotation->SetText(2, cmd.get<string>("input").c_str());
	int _MinSlice = imageViewer->GetSliceMin();
	int _MaxSlice = imageViewer->GetSliceMax();
	int _Slice = (_MaxSlice + _MinSlice) / 2;
	imageViewer->SetSlice(_Slice);
	std::stringstream tmp;
	tmp << "Slice Number  " << _Slice + 1 << "/" << _MaxSlice + 1;

	cornerAnnotation->SetText(1, tmp.str().c_str());
	cornerAnnotation->GetTextProperty()->SetColor(1, 0, 0);
	callback->SetAnnotation(cornerAnnotation);
	callback->SetCmdParser(&cmd);

	imageViewer->GetRenderer()->AddViewProp(cornerAnnotation);
	imageViewer->GetRenderWindow()->SetNumberOfLayers(2);
	imageViewer->GetRenderer()->SetViewport(0, 0, 1, 1);


	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->SetViewport(0, 0, 1, 0.3);
	renderer->SetLayer(1);
	imageViewer->GetRenderWindow()->AddRenderer(renderer);


	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	imageViewer->SetupInteractor(renderWindowInteractor);

	vtkSmartPointer<vtkUserInteractorStyleImage> myInteractorStyle =
		vtkSmartPointer<vtkUserInteractorStyleImage>::New();

	myInteractorStyle->SetImageViewer(imageViewer);
	myInteractorStyle->SetAnnotation(cornerAnnotation);
	myInteractorStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
	myInteractorStyle->AddObserver(vtkCommand::KeyPressEvent, callback);
	myInteractorStyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
	renderWindowInteractor->SetInteractorStyle(myInteractorStyle);

	vtkSmartPointer<vtkRegionSelectionWidget> regionSelectionWidget =
		vtkSmartPointer<vtkRegionSelectionWidget>::New();
	regionSelectionWidget->SetInteractor(renderWindowInteractor);
	regionSelectionWidget->CreateDefaultRepresentation();
	regionSelectionWidget->SelectableOff();
	regionSelectionWidget->SetViewer(imageViewer);
	regionSelectionWidget->SetRenderer(renderer);

	imageViewer->SetColorLevel(cmd.get<int>("level"));
	imageViewer->SetColorWindow(cmd.get<int>("window"));
	imageViewer->SetupInteractor(renderWindowInteractor);
	imageViewer->GetRenderWindow()->SetSize(800, 800);
	imageViewer->GetRenderer()->ResetCamera();
	renderWindowInteractor->Initialize();

	regionSelectionWidget->On();
	imageViewer->Render();
	renderWindowInteractor->Start();

}



//#include "itkBinaryThresholdImageFilter.h"
//#include "itkImageFileWriter.h"
//#include "itkSliceBySliceImageFilter.h"
//#include "itkConnectedComponentImageFilter.h"
//#include "itkLabelShapeKeepNObjectsImageFilter.h"
//
//#include "itkMedianImageFilter.h"
//#include "itkSubtractImageFilter.h"
//#include "itkImageFileReader.h"
//
//
//void main(int argc, char* argv[])
//{
//	typedef  unsigned short   PixelType;
//	typedef itk::Image< PixelType, 3 >   Image3DType;
//	typedef itk::Image< PixelType, 2 >   Image2DType;
//
//	typedef itk::ImageFileReader< Image3DType >  ReaderType;
//	typedef itk::ImageFileWriter< Image3DType >  WriterType;
//
//	typedef itk::ConnectedComponentImageFilter < Image2DType, Image2DType >
//		ConnectedComponentImageFilterType;
//	typedef itk::LabelShapeKeepNObjectsImageFilter< Image2DType >
//		LabelShapeKeepNObjectsImageFilterType;
//
//
//	ReaderType::Pointer reader = ReaderType::New();
//	reader->SetFileName("out1.gs.mha");
//	try
//	{
//		reader->Update();
//	}
//	catch (itk::ExceptionObject & excep)
//	{
//		std::cerr << "Exception caught !" << std::endl;
//		std::cerr << excep << std::endl;
//	}
//
//	ConnectedComponentImageFilterType::Pointer connectedComponentFilter = ConnectedComponentImageFilterType::New();
//	// connectedComponentFilter->SetInput()			//input from SliceBySliceImageFilter
//	// connectedComponentFilter->Update();			// should not use filter update
//
//	LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
//	labelShapeKeepNObjectsImageFilter->SetInput(connectedComponentFilter->GetOutput());
//	labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
//	labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
//	labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
//	// labelShapeKeepNObjectsImageFilter->Update();		// should not use filter update
//
//
//
//	typedef itk::SliceBySliceImageFilter< Image3DType, Image3DType>   SliceBySliceImageFilter;
//	SliceBySliceImageFilter::Pointer sliceBySliceFilter = SliceBySliceImageFilter::New();
//	sliceBySliceFilter->SetInput(reader->GetOutput());		// input
//	sliceBySliceFilter->SetInputFilter(connectedComponentFilter);			// the first filter of the image processing pipline
//	sliceBySliceFilter->SetOutputFilter(labelShapeKeepNObjectsImageFilter);	// the first filter of the image processing pipline
//	//sliceBySliceFilter->SetDimension(2);		// the reslice direction
//	try
//	{
//		sliceBySliceFilter->Update();
//	}
//	catch (itk::ExceptionObject & excep)
//	{
//		std::cerr << "Exception caught !" << std::endl;
//		std::cerr << excep << std::endl;
//	}
//	WriterType::Pointer writer = WriterType::New();
//	writer->SetInput(sliceBySliceFilter->GetOutput());
//	writer->SetFileName("out1.gsxx.mha");
//	try
//	{
//		writer->Update();
//	}
//	catch (itk::ExceptionObject & excep)
//	{
//		std::cerr << "Exception caught !" << std::endl;
//		std::cerr << excep << std::endl;
//	}
//}
//
