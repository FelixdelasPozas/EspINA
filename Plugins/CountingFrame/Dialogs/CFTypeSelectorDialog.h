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

#ifndef ESPINA_CF_CF_TYPE_SELECTOR_DIALOG_H
#define ESPINA_CF_CF_TYPE_SELECTOR_DIALOG_H

#include "CountingFramePlugin_Export.h"

// Plugin
#include "ui_CFTypeSelectorDialog.h"
#include <CountingFrames/CountingFrame.h>

// ESPINA
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/Proxies/ChannelProxy.h>

// Qt
#include <QDialog>

namespace ESPINA
{
  namespace Support
  {
    class Context;
  }

  namespace CF
  {
    class CountingFramePlugin_EXPORT CFTypeSelectorDialog
    : public QDialog
    , private Ui::CFTypeSelectorDialog
    {
        Q_OBJECT
      public:
        /** \brief CFTypeSelectorDialog class constructor.
         * \param[in] context application context.
         * \param[in] parent pointer of the widget parent of this one.
         *
         */
        CFTypeSelectorDialog(Support::Context &context, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget());

        /** \brief CFTypeSelectorDialog class virtual destructor.
         *
         */
        virtual ~CFTypeSelectorDialog()
        {};

        /** \brief Sets the type of counting frame to select.
         *
         */
        void setType(CFType type);

        /** \brief Returns the selected counting frame type.
         *
         */
        CFType type() const
        { return m_type; }

        /** \brief Returns the stack of the counting frame.
         *
         */
        ChannelAdapterPtr stack()
        { return m_stack; }

        /** \brief Returns the category constrint of the counting frame.
         *
         */
        QString categoryConstraint() const;

      public slots:
        void channelSelected();
        void radioChanged(bool);

      private:
        CFType                        m_type;   /** type of counting frame selected. */
        std::shared_ptr<ChannelProxy> m_proxy;  /** channel model proxy.             */

        ChannelAdapterPtr m_stack;      /** selected stack.                           */
        ModelAdapterSPtr  m_model;      /** qt model.                                 */
        QStringList       m_stackNames; /** list of names of the stacks in the model. */
        ModelFactorySPtr  m_factory;    /** factory for extension creation.           */
    };
  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_CF_CF_TYPE_SELECTOR_DIALOG_H
