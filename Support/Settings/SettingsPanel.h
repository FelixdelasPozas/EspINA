#ifndef ESPINA_ISETTINGSPANEL_H
#define ESPINA_ISETTINGSPANEL_H

#include "Support/EspinaSupport_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Support/Types.h>

// Qt
#include <QWidget>
#include <QIcon>

namespace ESPINA
{
  namespace Support
  {
    namespace Settings
    {
      /** \class SettingsPanel
       * \brief Settins panel super class.
       *
       */
      class EspinaSupport_EXPORT SettingsPanel
      : public QWidget
      {
        public:
          /** \brief SettingsPanel class virtual destructor.
           *
           */
          virtual ~SettingsPanel()
          {}

          /** \brief Returns a long description of what the panel provides.
           *
           */
          virtual const QString longDescription() = 0;

          /** \brief Returns a short description of what the panel provides (one or two words).
           *
           */
          virtual const QString shortDescription() = 0;

          /** \brief Returns the icon of the panel.
           *
           */
          virtual const QIcon icon() = 0;

          /** \brief Adds a sub-panel.
           * \param[in] panel raw pointer of the SettingsPanel object to add.
           *
           */
          virtual void addPanel(SettingsPanel *panel) {}

          /** \brief Changes values when the user accepts the modifications of the panel.
           *
           */
          virtual void acceptChanges()  = 0;

          /** \brief Reverts values to previous ones when the user rejects the modifications of the panel.
           *
           */
          virtual void rejectChanges()  = 0;

          /** \brief Returns true if the values of the panel have been modified.
           *
           */
          virtual bool modified() const = 0;

          /** \brief Returns a new instance of the settings panel.
           *
           */
          virtual SettingsPanelPtr clone() = 0;
      };
    } // namespace Settings
  } // namespace Support
} // namespace ESPINA

#endif// ISETTINGSPANEL_H
