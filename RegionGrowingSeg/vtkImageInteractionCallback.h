#ifndef VTK_IMAGE_INTERACTION_CALLBACK_H
#define VTK_IMAGE_INTERACTION_CALLBACK_H
#include "ImageProcessingFunc.h"

#include <vtkAssemblyPath.h>
#include <vtkCell.h>
#include <vtkCommand.h>
#include <vtkCornerAnnotation.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPointData.h>
#include <vtkPropPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkImageCast.h>
#include <vtkMath.h>
#include <vector>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>

#include "cmdline.h"
using namespace std;

template<typename T>
void vtkValueMessageTemplate(vtkImageData* image, int* position,
	std::string& message)
{
	T* tuple = ((T*)image->GetScalarPointer(position));
	int components = image->GetNumberOfScalarComponents();
	for (int c = 0; c < components; ++c)
	{
		message += vtkVariant(tuple[c]).ToString();
		if (c != (components - 1))
		{
			message += ", ";
		}
	}
	message += " )";
}


std::string Pick(vtkImageViewer2 *viewer, vtkPropPicker* picker, int *index = NULL)
{
	vtkRenderer*   renderer = viewer->GetRenderer();
	vtkImageActor* actor = viewer->GetImageActor();
	vtkImageData*  image = viewer->GetInput();
	vtkRenderWindowInteractor *interactor =
		viewer->GetRenderWindow()->GetInteractor();
	vtkInteractorStyle *style =
		vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());

	double* spacing = image->GetSpacing();
	double* origin = image->GetOrigin();

	// Pick at the mouse location provided by the interactor
	picker->Pick(interactor->GetEventPosition()[0],
		interactor->GetEventPosition()[1],
		0.0, renderer);
	// There could be other props assigned to this picker, so 
	// make sure we picked the image actor
	vtkAssemblyPath* path = picker->GetPath();
	bool validPick = false;
	//cout << interactor->GetEventPosition()[0] << "\t" << interactor->GetEventPosition()[1] << endl;

	if (path)
	{
		vtkCollectionSimpleIterator sit;
		path->InitTraversal(sit);
		vtkAssemblyNode *node;
		for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
		{
			node = path->GetNextNode(sit);
			if (actor == vtkImageActor::SafeDownCast(node->GetViewProp()))
			{
				validPick = true;
			}
		}
	}
	std::string message;//return message
	if (!validPick)
	{
		//·µ»ØOFF IMAGE
		message = "Off Image";
		interactor->Render();
		//style->OnMouseMove();
		return message;
	}

	// Get the world coordinates of the pick
	double pos[3];
	picker->GetPickPosition(pos);

	int image_coordinate[3];

	int axis = viewer->GetSliceOrientation();
	switch (axis)
	{
	case vtkImageViewer2::SLICE_ORIENTATION_XZ:
		image_coordinate[0] = vtkMath::Round(pos[0]);
		image_coordinate[1] = viewer->GetSlice();
		image_coordinate[2] = vtkMath::Round(pos[2]);
		break;
	case vtkImageViewer2::SLICE_ORIENTATION_YZ:
		image_coordinate[0] = viewer->GetSlice();
		image_coordinate[1] = vtkMath::Round((pos[0] - origin[0]) / spacing[1]);
		image_coordinate[2] = vtkMath::Round((pos[1] - origin[1]) / spacing[1]);
		break;
	default:  // vtkImageViewer2::SLICE_ORIENTATION_XY
		image_coordinate[0] = vtkMath::Round((pos[0] - origin[0]) / spacing[0]);
		image_coordinate[1] = vtkMath::Round((pos[1] - origin[1]) / spacing[1]);
		image_coordinate[2] = viewer->GetSlice();
		break;
	}

	if (index)
	{
		index[0] = image_coordinate[0];
		index[1] = image_coordinate[1];
		index[2] = image_coordinate[2];
	}
	message = "Location: ( ";
	message += vtkVariant(image_coordinate[0]).ToString();
	message += ", ";
	message += vtkVariant(image_coordinate[1]).ToString();
	message += ", ";
	message += vtkVariant(image_coordinate[2]).ToString();
	message += " )\nValue: ( ";

	
	switch (image->GetScalarType())
	{
		vtkTemplateMacro((vtkValueMessageTemplate<VTK_TT>(image,
			image_coordinate,
			message)));

	//default:
	//	return;
	}

	interactor->Render();
	style->OnMouseMove();
	return message;
}

class vtkImageInteractionCallback : public vtkCommand
{
public:
	static vtkImageInteractionCallback *New()
	{
		return new vtkImageInteractionCallback;
	}

	vtkImageInteractionCallback()
	{
		this->Viewer = NULL;
		this->Picker = NULL;
	}

	~vtkImageInteractionCallback()
	{
		this->Viewer = NULL;
		this->Picker = NULL;
	}

	void SetPicker(vtkPropPicker *picker)
	{
		this->Picker = picker;
	}

	void SetViewer(vtkImageViewer2 *viewer)
	{
		this->Viewer = viewer;
	}

	void SetCmdParser(cmdline::parser* cmd)
	{
		this->Cmd = cmd;
	}

	void SetAnnotation(vtkCornerAnnotation* annotation)
	{
		this->Annotation = annotation;
	}

	virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData)
	{
		switch (eventId)
		{
			case(vtkCommand::KeyPressEvent):
			{
				// Get the keypress
				vtkRenderWindowInteractor *rwi = this->Viewer->GetRenderWindow()->GetInteractor();
				std::string key = rwi->GetKeySym();

				// Do segmentation
				if (key == "Return")
				{
					cout << "Start Segmentation" << endl;
					DoSegmentation(Cmd->get<string>("input").c_str(), Cmd->get<string>("output").c_str(), KeysIndex, Cmd->get<int>("lower"), Cmd->get<int>("upper"));
					cout << "end Segmentation" << endl;
				}
				if (key == "r" || key == "R")
				{

				}
				break;
			}
			case(vtkCommand::MouseMoveEvent):
			{
				std::string msg = Pick(Viewer, Picker, NULL);
				Annotation->SetText(0, msg.c_str());
				break;
			}
			case(vtkCommand::LeftButtonPressEvent):
			{
				int index[3];
				std::string msg = Pick(Viewer, Picker, index);
				std::vector<int> vec;
				vec.push_back(index[0]);
				vec.push_back(index[1]);
				vec.push_back(index[2]);
				KeysIndex.push_back(vec);
				cout << "Pick Point:["<< index[0] << "," << index[1] << "," << index[2] << "]"<< endl;
				Annotation->SetText(0, msg.c_str());
				break;
			}

		}
	}

private:
	vtkImageViewer2*      Viewer;      // Pointer to the viewer
	vtkPropPicker*        Picker;      // Pointer to the picker
	vtkCornerAnnotation*  Annotation;
	std::vector<std::vector<int>>	 KeysIndex;
	cmdline::parser*	  Cmd;
};

#endif //VTK_IMAGE_INTERACTION_CALLBACK_H