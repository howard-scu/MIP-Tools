/*
Copyright (c) 2016, Lu Xiaohua
All rights reserved.
*/

#ifndef VTK_USER_INTERACTOR_STYLE_IMAGE_H
#define VTK_USER_INTERACTOR_STYLE_IMAGE_H
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <sstream>
#include <vtkCornerAnnotation.h>


// 
class StatusMessage {
public:
	static std::string Format(int slice, int maxSlice) {
		std::stringstream tmp;
		tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
		return tmp.str();
	}
};


// Define own interaction style
class vtkUserInteractorStyleImage : public vtkInteractorStyleImage
{
public:
	static vtkUserInteractorStyleImage* New();
	vtkTypeMacro(vtkUserInteractorStyleImage, vtkInteractorStyleImage);

protected:
	vtkImageViewer2* _ImageViewer;
	vtkCornerAnnotation* _Annotation;
	int _Slice;
	int _MinSlice;
	int _MaxSlice;

public:
	void SetImageViewer(vtkImageViewer2* imageViewer) {
		_ImageViewer = imageViewer;
		_MinSlice = imageViewer->GetSliceMin();
		_MaxSlice = imageViewer->GetSliceMax();
		_Slice = (_MaxSlice+ _MinSlice)/2;
		//cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << std::endl;
	}

	void SetAnnotation(vtkCornerAnnotation* annotation) {
		_Annotation = annotation;
	}


protected:
	void MoveSliceForward() {
		if (_Slice < _MaxSlice) {
			_Slice += 1;
			//cout << "MoveSliceForward::Slice = " << _Slice << std::endl;
			_ImageViewer->SetSlice(_Slice);
			std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
			_Annotation->SetText(1,msg.c_str());
			_ImageViewer->Render();
		}
	}

	void MoveSliceBackward() {
		if (_Slice > _MinSlice) {
			_Slice -= 1;
			//cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
			_ImageViewer->SetSlice(_Slice);
			std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
			_Annotation->SetText(1, msg.c_str());
			_ImageViewer->Render();
		}
	}


	virtual void OnKeyDown() {
		std::string key = this->GetInteractor()->GetKeySym();
		if (key.compare("Up") == 0) {
			//cout << "Up arrow key was pressed." << endl;
			MoveSliceForward();
		}
		else if (key.compare("Down") == 0) {
			//cout << "Down arrow key was pressed." << endl;
			MoveSliceBackward();
		}
		// forward event
		vtkInteractorStyleImage::OnKeyDown();
	}


	virtual void OnMouseWheelForward() {
		//std::cout << "Scrolled mouse wheel forward." << std::endl;
		MoveSliceForward();
		// don't forward events, otherwise the image will be zoomed 
		// in case another interactorstyle is used (e.g. trackballstyle, ...)
		// vtkInteractorStyleImage::OnMouseWheelForward();
	}


	virtual void OnMouseWheelBackward() {
		//std::cout << "Scrolled mouse wheel backward." << std::endl;
		if (_Slice > _MinSlice) {
			MoveSliceBackward();
		}
		// don't forward events, otherwise the image will be zoomed 
		// in case another interactorstyle is used (e.g. trackballstyle, ...)
		// vtkInteractorStyleImage::OnMouseWheelBackward();
	}
};

vtkStandardNewMacro(vtkUserInteractorStyleImage);

#endif //VTK_USER_INTERACTOR_STYLE_IMAGE_H