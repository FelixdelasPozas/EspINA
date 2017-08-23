/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_GUI_REPRESENTATIONS_FRAME_H
#define ESPINA_GUI_REPRESENTATIONS_FRAME_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "GUI/Types.h"
#include "GUI/Utils/Timer.h"
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Vector3.hxx>

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      struct EspinaGUI_EXPORT Frame
      {
        public:
          /** \brief Frame struct constructor.
           *
           */
          explicit Frame();

          /** \brief Frame struct destructor.
           *
           */
          ~Frame();

          /** \brief Returns an invalid frame.
           *
           */
          static FrameCSPtr InvalidFrame();

          enum Option
          {
            None = 0x0, Focus = 0x1, Reset = 0x2, InvalidateSegmentation = 0x4, InvalidateChannel = 0x8
          };

          Q_DECLARE_FLAGS(Options, Option)

          TimeStamp time;
          NmVector3 crosshair;
          NmVector3 resolution;
          Bounds    bounds;
          Options   flags;
      };

      /** \brief QDebug stream operator<< for a Frame (for debug purposes).
       *
       */
      QDebug EspinaGUI_EXPORT operator<<(QDebug d, const FrameSPtr frame);

      /** \brief QDebug stream operator<< for a const Frame (for debug purposes).
       *
       */
      QDebug EspinaGUI_EXPORT operator<<(QDebug d, const FrameCSPtr frame);

      /** \brief Returns true if the frame is valid (timestamp is valid).
      *
      */
      bool EspinaGUI_EXPORT isValid(const FrameCSPtr frame);

      /** \brief Returns true if the frame requires a camera reset.
       *
       */
      bool EspinaGUI_EXPORT requiresReset(const FrameCSPtr frame);

      /** \brief Returns true if the frame requires a focus on the frame crosshair.
       *
       */
      bool EspinaGUI_EXPORT requiresFocus(const FrameCSPtr frame);

      /** \brief Returns true if the frame invalidates segmentation representations.
       *
       */
      bool EspinaGUI_EXPORT invalidatesSegmentations(const FrameCSPtr frame);

      /** \brief Returns true if the frame invalidates channels representations.
       *
       */
      bool EspinaGUI_EXPORT invalidatesChannels(const FrameCSPtr frame);
    }
  }
}

#endif // ESPINA_GUI_REPRESENTATIONS_FRAME_H
