/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ADAPTIVEEDGESDIALOG_H_
#define ADAPTIVEEDGESDIALOG_H_

// ESPINA
#include <ui_AdaptiveEdgesDialog.h>

// Qt
#include <QDialog>

namespace ESPINA
{

  class AdaptiveEdgesDialog
  : public QDialog
  , private Ui::AdaptiveEdgesDialog
  {
    Q_OBJECT

    public:
			/** brief AdaptiveEdgesDialog class constructor.
			 * \param[in] parent, Parent widget.
			 */
      AdaptiveEdgesDialog(QWidget *parent);

      /** brief AdaptiveEdgesDialog class destructor.
       *
       */
      virtual ~AdaptiveEdgesDialog()
      {};

      /** brief Returns the threshold used to classify a voxel as background or not.
       *
       */
      int threshold()         { return m_threshold; }

      /** brief Returns the value used as background color.
       *
       */
      int color()             { return m_backgroundColor; }

      /** brief Returns true if adaptive edges is enabled.
       *
       */
      bool useAdaptiveEdges() { return m_adaptiveEdgesEnabled; }

    private slots:
      void radioChanged(bool);
      void bgColorChanged(int value);
      void thresholdChanged(int value);
      void setBlackBgColor();
      void setWhiteBgColor();

    private:
      bool m_adaptiveEdgesEnabled;
      int m_backgroundColor;
      int m_threshold;
  };

} /* namespace ESPINA */
#endif /* ADAPTIVEEDGESDIALOG_H_ */
