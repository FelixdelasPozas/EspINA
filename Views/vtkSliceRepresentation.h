/*
   <one line to give the program's name and a brief idea of what it does.>
   Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//----------------------------------------------------------------------------
// File:    vtkSliceRepresentation.h
// Purpose: Transform source's vtkImageData into slice representations to
//          be rendered in vtkPVEspinaViews
//          vtkSliceRepresentation is not meant to be used with
//          conventional vtkPVViews
//----------------------------------------------------------------------------
#ifndef VTKSLICEREPRESENTATION_H
#define VTKSLICEREPRESENTATION_H

#include <vtkPVDataRepresentation.h>
#include "vtkPVSliceView.h"

class vtkImageResliceToColors;
class vtkImageActor;
class vtkImageSliceDataDeliveryFilter;
class vtkImageSlice;
class vtkImageProperty;
class vtkImageResliceMapper;


class vtkSliceRepresentation : public vtkPVDataRepresentation
{
	public:
		static vtkSliceRepresentation* New();
		vtkTypeMacro ( vtkSliceRepresentation,vtkPVDataRepresentation );

		// vtkAlgorithm::ProcessRequest() equivalent for rendering passes. This is
		// typically called by the vtkView to request meta-data from the
		// representations or ask them to perform certain tasks e.g.
		// PrepareForRendering.
		virtual int ProcessViewRequest ( vtkInformationRequestKey* request_type, vtkInformation* inInfo, vtkInformation* outInfo );

		void SetType ( int value );
		vtkGetMacro ( Type,int );
		//BTX
	protected:
		vtkSliceRepresentation();
		virtual ~vtkSliceRepresentation();

		// Fill input port information
		virtual int FillInputPortInformation ( int port, vtkInformation* info );

		// Subclasses should override this to connect inputs to the internal pipeline
		// as necessary. Since most representations are "meta-filters" (i.e. filters
		// containing other filters), you should create shallow copies of your input
		// before connecting to the internal pipeline. The convenience method
		// GetInternalOutputPort will create a cached shallow copy of a specified
		// input for you. The related helper functions GetInternalAnnotationOutputPort,
		// GetInternalSelectionOutputPort should be used to obtain a selection or
		// annotation port whose selections are localized for a particular input data object.
		virtual int RequestData ( vtkInformation* , vtkInformationVector** , vtkInformationVector* );

		virtual bool AddToView ( vtkView* view );
		virtual void SetVisibility(bool val);

		vtkTimeStamp DeliveryTimeStamp;

		vtkImageSliceDataDeliveryFilter *DeliveryFilter;
		vtkImageResliceToColors  *Slice;
		vtkImageActor            *SliceActor;
		vtkImageData             *SliceData;
		vtkPVSliceView::SegActor SegActor;
	private:
		int Type;
		//ETX
};

#endif // VTKSLICEREPRESENTATION_H
