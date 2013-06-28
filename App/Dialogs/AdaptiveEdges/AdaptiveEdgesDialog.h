/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef ADAPTIVEEDGESDIALOG_H_
#define ADAPTIVEEDGESDIALOG_H_

// EspINA
#include <ui_AdaptiveEdgesDialog.h>

// Qt
#include <QDialog>

namespace EspINA
{
  
  class AdaptiveEdgesDialog
  : public QDialog
  , private Ui::AdaptiveEdgesDialog
  {
    Q_OBJECT

    public:
      AdaptiveEdgesDialog(QWidget *parent);
      virtual ~AdaptiveEdgesDialog() {};

      int threshold()         { return m_threshold; }
      int color()             { return m_backgroundColor; }
      bool useAdaptiveEdges() { return m_adaptiveEdgesEnabled; }

    public slots:
      void radioChanged(bool);
      void bgColorChanged(int value);
      void thresholdChanged(int value);

    private:
      bool m_adaptiveEdgesEnabled;
      int m_backgroundColor;
      int m_threshold;
  };

} /* namespace EspINA */
#endif /* ADAPTIVEEDGESDIALOG_H_ */
