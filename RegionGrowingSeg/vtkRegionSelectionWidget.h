/*
Copyright (c) 2016, Lu Xiaohua
All rights reserved.
*/

#ifndef VTK_REGION_SELECTION_WIDGET_H
#define VTK_REGION_SELECTION_WIDGET_H

#include <vtkSmartPointer.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCommand.h>
#include <vtkWidgetEvent.h>
#include <vtkObjectFactory.h>
#include <vtkActor.h>
#include <vtkBorderRepresentation.h>
#include <vtkBorderWidget.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageViewer2.h>
#include <vtkPropPicker.h>
#include <vtkImageAccumulate.h>
#include <vtkImageStencilData.h>
#include <vtkExtractVOI.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageMask.h>
#include <vtkXYPlotActor.h>


class vtkRegionSelectionWidget : public vtkBorderWidget
{
public:
	static vtkRegionSelectionWidget *New();
	vtkTypeMacro(vtkRegionSelectionWidget, vtkBorderWidget);

	void SetViewer(vtkImageViewer2 *viewer)
	{
		this->Viewer = viewer;
	}

	void SetRenderer(vtkRenderer *viewer)
	{
		this->Renderer = viewer;
	}
	
	int SubclassEndSelectAction()
	{
		vtkImageData*  image = Viewer->GetInput();
	
		double* spacing = image->GetSpacing();
		double* origin = image->GetOrigin();
		int* extent = image->GetExtent();
		int slice = Viewer->GetSlice();
		int	nlowerLeft[2];
		int	nupperRight[2];
		double* lowerLeft = static_cast<vtkBorderRepresentation*>(this->GetRepresentation())->GetPositionCoordinate()->GetComputedWorldValue(Viewer->GetRenderer());
		nlowerLeft[0] = (int)((lowerLeft[0] - origin[0]) / spacing[0]);
		nlowerLeft[1] = (int)((lowerLeft[1] - origin[1]) / spacing[1]);
		double* upperRight = static_cast<vtkBorderRepresentation*>(this->GetRepresentation())->GetPosition2Coordinate()->GetComputedWorldValue(Viewer->GetRenderer());
		nupperRight[0] = (int)((upperRight[0] - origin[0]) / spacing[0]);
		nupperRight[1] = (int)((upperRight[1] - origin[1]) / spacing[1]);

		extractVOI->SetInputData(image);
		extractVOI->SetVOI(nlowerLeft[0], nupperRight[0], nlowerLeft[1], nupperRight[1], slice, slice);
		extractVOI->Update();

		histogram->SetInputData(extractVOI->GetOutput());
		//histogram->SetComponentExtent(0, 300, 0, 0, 0, 0);
		histogram->SetComponentOrigin(0, 0, 0);
		histogram->SetComponentSpacing(30, 0, 0);
		histogram->IgnoreZeroOff();
		histogram->Update();
		cout << "========================" << endl;
		cout << "  Area:  [ " << histogram->GetVoxelCount()*spacing[0] * spacing[1] << " ]" << endl;
		cout << "  Mean:  [ " << *(histogram->GetMean()) << " ]" << endl;
		cout << "Min/Max: [ " << *(histogram->GetMin()) << ", " << *(histogram->GetMax()) << " ]" << endl;
		cout << "Std.Dev: [ " << *(histogram->GetStandardDeviation()) << " ]" << endl;
		cout << "========================" << endl;

		vtkSmartPointer<vtkXYPlotActor> plot =
			vtkSmartPointer<vtkXYPlotActor>::New();
		plot->ExchangeAxesOff();
		plot->SetLabelFormat("%g");
		//plot->SetXTitle("Level");
		//plot->SetYTitle("Frequency");
		plot->SetXValuesToValue();


		plot->AddDataSetInputConnection(histogram->GetOutputPort());
		plot->SetPlotColor(0, 1, 1, 1);
		plot->LegendOn();

		plot->SetXRange(0, extractVOI->GetOutput()->GetScalarRange()[1]);
		plot->SetYRange(0, histogram->GetOutput()->GetScalarRange()[1]);

		Renderer->RemoveAllViewProps();
		Renderer->AddActor(plot);
		Viewer->Render();

		return vtkBorderWidget::SubclassSelectAction(); // works
	}

	vtkRegionSelectionWidget()
	{
		this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
			vtkWidgetEvent::EndSelect,
			this, vtkRegionSelectionWidget::EndSelectAction);
		histogram = vtkSmartPointer<vtkImageAccumulate>::New();
		extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
	}
	vtkImageViewer2*      Viewer;
	vtkSmartPointer<vtkImageAccumulate> histogram;
	vtkSmartPointer<vtkExtractVOI> extractVOI;
	vtkRenderer*      Renderer;
};

vtkStandardNewMacro(vtkRegionSelectionWidget);

#endif  // VTK_REGION_SELECTION_WIDGET_H
