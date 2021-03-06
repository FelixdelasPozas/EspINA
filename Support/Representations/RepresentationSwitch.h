/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_REPRESENTATION_SWITCH_H
#define ESPINA_REPRESENTATION_SWITCH_H

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/Utils/Timer.h>
#include <Support/Widgets/ProgressTool.h>
#include <memory>
#include <QWidget>

namespace ESPINA
{
  /** \class RepresentationSwitch
   * \brief Tool button implementation for representation switches.
   *
   */
  class EspinaSupport_EXPORT RepresentationSwitch
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT
  public:
    /** \brief RepresentationSwitch class virtual destructor.
     *
     */
    virtual ~RepresentationSwitch()
    {}

    /** \brief Returns the flags of the supported views of the managers.
     *
     */
    virtual ViewTypeFlags supportedViews() const = 0;

    /** \brief Shows the managed representations on the view.
     *
     */
    virtual void showRepresentations(const GUI::Representations::FrameCSPtr frame) = 0;

    /** \brief Hides the managed representations from the view.
     *
     */
    virtual void hideRepresentations(const GUI::Representations::FrameCSPtr frame) = 0;

    /** \brief Invalidates the representations of the given items.
     * \param[in] items list of ViewItemAdapter.
     *
     */
    void invalidateRepresentations(ViewItemAdapterList items);

  protected:
    /** \brief RepresentationSwitch class constructor.
     * \param[in] id id of the switch.
     * \param[in] icon icon to show.
     * \param[in] description switch description.
     * \param[in] timer timer to use for operations.
     * \param[in] context session context.
     *
     */
    explicit RepresentationSwitch(const QString    &id,
                                  const QIcon      &icon,
                                  const QString    &description,
                                  Support::Context &context);

  private slots:
    /** \brief Switches representation visibility.
     *
     */
    void switchRepresentations(bool show);

  private:
    /** \brief Invalidates the representations of the given items.
     * \param[in] items list of ViewItemAdapter.
     *
     */
    virtual void invalidateRepresentationsImplementation(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame) = 0;
  };

  using RepresentationSwitchSPtr  = std::shared_ptr<RepresentationSwitch>;
  using RepresentationSwitchSList = QList<RepresentationSwitchSPtr>;
}

#endif // ESPINA_REPRESENTATION_SWITCH_H
