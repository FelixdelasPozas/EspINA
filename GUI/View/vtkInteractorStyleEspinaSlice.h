/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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


#ifndef VTKINTERACTORSTYLEESPINASLICE_H
#define VTKINTERACTORSTYLEESPINASLICE_H

#include "GUI/EspinaGUI_Export.h"

// VTK
#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

// Interactor Style to be used with Slice Views
class EspinaGUI_EXPORT vtkInteractorStyleEspinaSlice
: public vtkInteractorStyleImage
{
	public:
		static vtkInteractorStyleEspinaSlice *New();
		vtkTypeMacro(vtkInteractorStyleEspinaSlice, vtkInteractorStyleImage);

		/** brief Disable mouse wheel.
		 *
		 */
		virtual void OnMouseWheelForward() override
		{}

		/** brief Disable mouse wheel.
		 *
		 */
		virtual void OnMouseWheelBackward() override
		{}

		/** brief Disable keyboard.
		 *
		 */
		virtual void OnKeyPress() override
		{}

		/** brief Disable keyboard.
		 *
		 */
		virtual void OnKeyRelease() override
		{}

		/** brief Disable keyboard.
		 *
		 */
		virtual void OnKeyUp() override
		{}

		/** brief Disable keyboard.
		 *
		 */
		virtual void OnKeyDown() override
		{}

		/** brief Disable mdifications of brightness and saturation.
		 *
		 */
		virtual void OnLeftButtonDown() override
		{}

		/** brief Disable mdifications of brightness and saturation.
		 *
		 */
		virtual void OnLeftButtonUp() override
		{}

		/** brief Disable zoom if ctrl is pressed.
		 *
		 */
		virtual void OnRightButtonDown();

	protected:
		/** brief vtkInteractorStyleEspinaSlice class constructor.
		 *
		 */
		explicit vtkInteractorStyleEspinaSlice()
		{
			AutoAdjustCameraClippingRangeOn();
			KeyPressActivationOff();
		}

		/** brief vtkInteractorStyleEspinaSlice class destructor.
		 *
		 */
		virtual ~vtkInteractorStyleEspinaSlice()
		{}

	private:
		vtkInteractorStyleEspinaSlice(const vtkInteractorStyleEspinaSlice&); // Not implemented
		void operator=(const vtkInteractorStyleEspinaSlice&);// Not implemented
};

using View2DInteractor = vtkSmartPointer<vtkInteractorStyleEspinaSlice>;

#endif // VTKINTERACTORSTYLEESPINASLICE_H
