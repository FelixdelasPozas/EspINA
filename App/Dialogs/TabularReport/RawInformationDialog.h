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


#ifndef ESPINA_RAW_INFORMATION_DIALOG_H
#define ESPINA_RAW_INFORMATION_DIALOG_H

#include <QDialog>

#include <Core/EspinaTypes.h>
#include <EspinaConfig.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

namespace EspINA
{

  class RawInformationDialog
  : public QDialog
  {
  public:
    explicit RawInformationDialog(ModelAdapterSPtr model,
                                  ViewManagerSPtr  viewManager,
                                  QWidget         *parent = 0);
    virtual ~RawInformationDialog();

  protected:
    virtual void closeEvent(QCloseEvent *event);
  };

} // namespace EspINA

#endif // ESPINA_RAW_INFORMATION_DIALOG_H
