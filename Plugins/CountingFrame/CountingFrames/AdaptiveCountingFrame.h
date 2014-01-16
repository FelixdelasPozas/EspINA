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

#ifndef ADAPTIVEBOUNDINGFRAME_H
#define ADAPTIVEBOUNDINGFRAME_H

#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"

#include <Core/Utils/Bounds.h>

namespace EspINA
{
  namespace CF
  {
    const QString ADAPTIVE_CF = QObject::tr("Adaptive CF");

    class CountingFramePlugin_EXPORT AdaptiveCountingFrame
    : public CountingFrame
    {
    public:
      static AdaptiveCountingFrame *New(Id id,
                                        CountingFrameExtension *extension,
                                        const Bounds &bounds,
                                        Nm inclusion[3],
                                        Nm exclusion[3])
      { return new AdaptiveCountingFrame(id, extension, bounds, inclusion, exclusion); }

      virtual ~AdaptiveCountingFrame();

      // Implements QStandardItem interface
      virtual QVariant data(int role = Qt::UserRole + 1) const;
      virtual QString name() const { return ADAPTIVE_CF; }

      // Implements EspinaWidget itnerface
      virtual vtkAbstractWidget *create3DWidget(View3D *view);

      virtual SliceWidget *createSliceWidget(View2D *view);

      virtual bool processEvent(vtkRenderWindowInteractor* iren,
                                long unsigned int event);

      virtual void setEnabled(bool enable);

      virtual void updateCountingFrameImplementation();

    protected:
      explicit AdaptiveCountingFrame(Id id,
                                     CountingFrameExtension *extension,
                                     const Bounds &bounds,
                                     Nm inclusion[3],
                                     Nm exclusion[3]);

    protected:
      Nm leftOffset()   const {return  m_inclusion[0];}
      Nm topOffset()    const {return  m_inclusion[1];}
      Nm frontOffset()  const {return  m_inclusion[2];}
      Nm rightOffset()  const {return -m_exclusion[0];}
      Nm bottomOffset() const {return -m_exclusion[1];}
      Nm backOffset()   const {return -m_exclusion[2];}

      void applyOffset(double &var, double offset)
      {var = floor(var + offset + 0.5);}

    private:
      Channel *m_channel;
    };
  } // namespace CF
} // namespace EspINA

#endif // ADAPTIVEBOUNDINGFRAME_H
