/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CF_CF_TYPE_SELECTOR_DIALOG_H
#define ESPINA_CF_CF_TYPE_SELECTOR_DIALOG_H

// EspINA
#include "ui_CFTypeSelectorDialog.h"

#include <CountingFrames/CountingFrame.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/Proxies/ChannelProxy.h>

// Qt
#include <QDialog>

namespace EspINA
{
  namespace CF
  {
    class CFTypeSelectorDialog
    : public QDialog
    , private Ui::CFTypeSelectorDialog
    {
      Q_OBJECT

    public:
      CFTypeSelectorDialog(ModelAdapterSPtr model, QWidget *parent);

      virtual ~CFTypeSelectorDialog() {};

      void setType(CFType type);

      CFType type() const { return m_type; }

      ChannelAdapterPtr channel()
      { return m_channel; }

      QString categoryConstraint() const;

    public slots:
      void channelSelected();

      void radioChanged(bool);

    private:
      CFType m_type;

      std::shared_ptr<ChannelProxy> m_proxy;

      QModelIndex       m_channelIndex;
      ChannelAdapterPtr m_channel;
    };

  }
} /* namespace EspINA */

#endif // ESPINA_CF_CF_TYPE_SELECTOR_DIALOG_H
