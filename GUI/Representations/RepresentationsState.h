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

#ifndef ESPINA_REPRESENTATION_STATE_LIST_H
#define ESPINA_REPRESENTATION_STATE_LIST_H

#include <GUI/Model/ViewItemAdapter.h>

namespace ESPINA
{
  class RepresentationsState
  : public QObject
  {
    Q_OBJECT

  signals:
    void representationAdded();
    void representationUpdated();
    void representationRemoved();

    void visibilityChanged();
    void outputChanged();
  };

  using RepresentationsStateSPtr  = std::shared_ptr<RepresentationsState>;
  using RepresentationsStateSList = QList<RepresentationsStateSPtr>;

  template<typename T>
  class StateList
  : public RepresentationsState
  {
  public:
    void addRepresentation(typename T::Item item);

    bool updateRepresentation(typename T::Item item);

    void removeRepresentation(typename T::Item item);

    void insert(typename T::Item item)
    { m_states.insert(item, T(item)); }

    bool contains(typename T::Item item) const
    { return m_states.contains(item); }

    void remove(typename T::Item item)
    { m_states.remove(item); }

  private:
    QMap<typename T::Item, T> m_states;
  };


  //-----------------------------------------------------------------------------
  template<typename T>
  void StateList<T>::addRepresentation(typename T::Item item)
  {
    Q_ASSERT(!contains(item));

    item->output()->update();

    //TODO: se podria conectar directamente a la señal del m_channelStates representationUpdated
    //   connect(channel, SIGNAL(outputChanged(ViewItemAdapterPtr)),
    //           this,    SLOT(changedOutput(ViewItemAdapterPtr)));
    insert(item);

    updateRepresentation(item /*, false*/);
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  bool StateList<T>::updateRepresentation(typename T::Item item)
  {
    if (!contains(item))
    {
      qWarning() << "Update Representation on non-registered channel";
      return false;
    }

    Q_ASSERT(contains(item));

    return m_states.value(item).updateState();
  }

  //-----------------------------------------------------------------------------
  template<typename T>
  void StateList<T>::removeRepresentation(typename T::Item item)
  {
    Q_ASSERT(contains(item));

    //   for(auto representation: m_channelStates[channel].representations)
    //   {
    //     for(auto renderer: m_renderers)
    //     {
    //       if (renderer->type() == Renderer::Type::Representation)
    //       {
    //         auto repRenderer = representationRenderer(renderer);
    //         if (repRenderer->hasRepresentation(representation))
    //         {
    //           repRenderer->removeRepresentation(representation);
    //         }
    //       }
    //     }
    //   }


    //   disconnect(channel, SIGNAL(outputChanged(ViewItemAdapterPtr)),
    //              this,    SLOT(changedOutput(ViewItemAdapterPtr)));
    remove(item);
  }
}

#endif // ESPINA_REPRESENTATION_STATE_LIST_H
