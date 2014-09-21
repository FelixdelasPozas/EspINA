/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_RAW_INFORMATION_DIALOG_H
#define ESPINA_RAW_INFORMATION_DIALOG_H

// Qt
#include <QDialog>

// ESPINA
#include <Core/EspinaTypes.h>
#include <EspinaConfig.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

namespace ESPINA
{

  class RawInformationDialog
  : public QDialog
  {
  public:
  	/** brief RawInformationDialog class constructor.
  	 * \param[in] model, model adapter smart pointer.
  	 * \param[in] factory, factory smart pointer.
  	 * \param[in] viewmanager, view manager smart pointer.
  	 * \param[in] parent, parent widget.
  	 */
    explicit RawInformationDialog(ModelAdapterSPtr model,
                                  ModelFactorySPtr factory,
                                  ViewManagerSPtr  viewManager,
                                  QWidget         *parent = nullptr);

    /** brief RawInformationDialog class destructor.
     *
     */
    virtual ~RawInformationDialog();

  protected:
    virtual void closeEvent(QCloseEvent *event) override;
  };

} // namespace ESPINA

#endif // ESPINA_RAW_INFORMATION_DIALOG_H
