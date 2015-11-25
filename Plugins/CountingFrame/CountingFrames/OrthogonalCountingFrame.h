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

#include "CountingFramePlugin_Export.h"

// Plugin
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
        /** \brief OrthogonalCountingFrame static vtk-style New constructor.
         * \param[in] extension extension of this CF.
         * \param[in] inclusion inclusion margins.
         * \param[in] exclusion exclusion margins.
         * \param[in] scheduler task scheduler.
         *
         */
        static OrthogonalCountingFrame *New(CountingFrameExtension *extension,
                                            Nm inclusion[3],
                                            Nm exclusion[3],
                                            SchedulerSPtr scheduler)
        { return new OrthogonalCountingFrame(extension, inclusion, exclusion, scheduler);}

        /** \brief OrthogonalCountingFrame class virtual destructor.
         *
         */
        virtual ~OrthogonalCountingFrame();

        virtual CFType cfType() const
        { return CF::ORTOGONAL; }

        virtual QString typeName() const { return ORTOGONAL_CF; }

        virtual void updateCountingFrameImplementation();

      protected:
        /** \brief OrthogonalCountingFrame class constructor.
         * \param[in] extension extension of this CF.
         * \param[in] inclusion inclusion margins.
         * \param[in] exclusion exclusion margins.
         * \param[in] scheduler task scheduler.
         *
         */
        explicit OrthogonalCountingFrame(CountingFrameExtension *extension,
                                        Nm inclusion[3],
                                        Nm exclusion[3],
                                        SchedulerSPtr scheduler);

        /** \brief Returns a vtkPolyData prism with the given limits.
         * \param[in] left left side limit in Nm.
         * \param[in] top top side limit in Nm.
         * \param[in] front front side limit in Nm.
         * \param[in] right right side limit in Nm.
         * \param[in] bottom bottom side limit in Nm.
         * \param[in] back back side limit in Nm.
         */
        vtkSmartPointer<vtkPolyData> createRectangularRegion(Nm left,
                                                             Nm top,
                                                             Nm front,
                                                             Nm right,
                                                             Nm bottom,
                                                             Nm back);

      private:
        Bounds m_bounds; /** bounds of the extension's extended item. */

        friend class vtkCountingFrameCommand;
    };

  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_CF_ORTOGONAL_COUNTING_FRAME_H
