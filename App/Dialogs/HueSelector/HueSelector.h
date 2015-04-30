/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_HUESELECTOR_HUESELECTOR_H_
#define APP_DIALOGS_HUESELECTOR_HUESELECTOR_H_

// ESPINA
#include <ui_HueSelectorDialog.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Widgets/HueSelector.h>

#include <QDialog>

namespace ESPINA
{
  class HueSelectorDialog
  : public QDialog
  , public Ui::HueSelectorDialog
  {
      Q_OBJECT
    public:
      HueSelectorDialog(const Hue value);
      virtual ~HueSelectorDialog();

      Hue hueValue() const;

    private slots:
      void onSelectorValueChanged(int h, int s, int v);
      void onSpinboxValueChanged(int h);

    private:
      Hue m_hue;
      HueSelector *m_selector;
  };
    
} // namespace ESPINA

#endif // APP_DIALOGS_HUESELECTOR_HUESELECTOR_H_
