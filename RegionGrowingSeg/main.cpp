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
#include "vtkSeedCallback .h"
#include "vtkUserInteractorStyleImage.h"

#include "cmdline.h"
#include <Winbase.h>

void main(int argc, char* argv[])
{
	cmdline::parser cmd;
	cmd.add<string>("input", 'f', "input filename",true);
	cmd.add<string>("output", 'o', "output filename",true);
	cmd.add<int>("upper", 'u', "upper threshold",true);
	cmd.add<int>("lower", 'l', "lower threshold",true);
	cmd.add<int>("window", 'w', "display window width",1500);
	cmd.add<int>("level", 'c', "display window level",3000);
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

	imageViewer->SetColorLevel(cmd.get<int>("level"));
	imageViewer->SetColorWindow(cmd.get<int>("window"));
	imageViewer->SetupInteractor(renderWindowInteractor);
	imageViewer->GetRenderWindow()->SetSize(800, 600);
	imageViewer->Render();
	imageViewer->GetRenderer()->ResetCamera();
	renderWindowInteractor->Initialize();
	renderWindowInteractor->Start();

}
