/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_CF_ORTOGONAL_COUNTING_FRAME_H
#define ESPINA_CF_ORTOGONAL_COUNTING_FRAME_H

#include "CountingFramePlugin_Export.h"

#include "Plugins/CountingFrame/CountingFrames/CountingFrame.h"

namespace EspINA
{
  namespace CF {
  class CountingFrameExtension;

  const QString ORTOGONAL_CF = QObject::tr("Ortogonal");

  class CountingFramePlugin_EXPORT OrtogonalCountingFrame
  : public CountingFrame
  {
  public:
    static OrtogonalCountingFrame *New(CountingFrameExtension *extension,
                                         const Bounds &bounds,
                                         Nm inclusion[3],
                                         Nm exclusion[3])
    { return new OrtogonalCountingFrame(extension, bounds, inclusion, exclusion);}

    virtual ~OrtogonalCountingFrame();

    virtual CFType cfType() const
    { return CF::ORTOGONAL; }

    virtual QString name() const { return ORTOGONAL_CF; }

    // Implements EspinaWidget interface
    virtual vtkAbstractWidget *create3DWidget(View3D *view);

    virtual SliceWidget *createSliceWidget(View2D *view);

    virtual bool processEvent(vtkRenderWindowInteractor* iren,
                              long unsigned int event);

    virtual void setEnabled(bool enable);

    virtual void updateCountingFrameImplementation();

  protected:
    explicit OrtogonalCountingFrame(CountingFrameExtension *extension,
                                      const Bounds &bounds,
                                      Nm inclusion[3],
                                      Nm exclusion[3]);

    vtkSmartPointer<vtkPolyData> createRectangularRegion(Nm left,
                                                         Nm top,
                                                         Nm front,
                                                         Nm right,
                                                         Nm bottom,
                                                         Nm back);

  private:
    Bounds m_bounds;
  };

  } // namespace CF
} // namespace EspINA

#endif // ESPINA_CF_ORTOGONAL_COUNTING_FRAME_H
