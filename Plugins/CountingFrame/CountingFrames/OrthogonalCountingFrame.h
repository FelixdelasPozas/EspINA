/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"

namespace ESPINA
{
  namespace CF
  {

  class vtkCountingFrameCommand;
  class CountingFrameExtension;

  const QString ORTOGONAL_CF = QObject::tr("Orthogonal");

  class CountingFramePlugin_EXPORT OrthogonalCountingFrame
  : public CountingFrame
  {
  public:
    static OrthogonalCountingFrame *New(CountingFrameExtension *extension,
                                        const Bounds &bounds,
                                        Nm inclusion[3],
                                        Nm exclusion[3],
                                        SchedulerSPtr scheduler)
    { return new OrthogonalCountingFrame(extension, bounds, inclusion, exclusion, scheduler);}

    virtual ~OrthogonalCountingFrame();

    virtual CFType cfType() const
    { return CF::ORTOGONAL; }

    virtual QString typeName() const { return ORTOGONAL_CF; }

    virtual void updateCountingFrameImplementation();

  protected:
    explicit OrthogonalCountingFrame(CountingFrameExtension *extension,
                                    const Bounds &bounds,
                                    Nm inclusion[3],
                                    Nm exclusion[3],
                                    SchedulerSPtr scheduler);

    vtkSmartPointer<vtkPolyData> createRectangularRegion(Nm left,
                                                         Nm top,
                                                         Nm front,
                                                         Nm right,
                                                         Nm bottom,
                                                         Nm back);

  private:
    Bounds m_bounds;

    friend class vtkCountingFrameCommand;
  };

  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_CF_ORTOGONAL_COUNTING_FRAME_H
