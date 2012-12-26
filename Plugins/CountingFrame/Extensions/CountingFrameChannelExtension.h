/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef COUNTINGFRAMECHANNELEXTENSION_H
#define COUNTINGFRAMECHANNELEXTENSION_H

#include <Core/Extensions/ChannelExtension.h>
#include <CountingFrames/CountingFrame.h>

class CountingFramePanel;
class ViewManager;

class CountingFrameChannelExtension
: public ChannelExtension
{
  Q_OBJECT
public:
  static const ExtId ID;
  static const ExtId ID_1_2_5; //Backwards compatibility
  static const ModelItem::ArgumentId COUNTING_FRAMES;

  explicit CountingFrameChannelExtension(CountingFramePanel *plugin, ViewManager *vm);
  virtual ~CountingFrameChannelExtension();

  virtual ModelItemExtension::ExtId id()
  { return ID; }

  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());
  virtual QString serialize() const;

  virtual ModelItemExtension::ExtIdList dependencies() const;

  virtual ModelItemExtension::InfoList availableInformations() const
  { return ChannelExtension::availableInformations(); }

  virtual ModelItemExtension::RepList availableRepresentations() const
  { return ChannelExtension::availableRepresentations(); }

  virtual QVariant information(ModelItemExtension::InfoTag tag) const
  { return ChannelExtension::information(tag); }

  virtual ChannelExtension* clone();

  void addCountingFrame(CountingFrame* countingFrame);
  void deleteCountingFrame(CountingFrame *countingFrame);
  CountingFrameList countingFrames() const {return m_countingFrames;}

protected slots:
  void countinfFrameUpdated(CountingFrame* countingFrame);

private:
  CountingFramePanel *m_plugin;
  ViewManager        *m_viewManager;
  CountingFrameList   m_countingFrames;

  mutable ModelItem::Arguments    m_args;
};

#endif // COUNTINGFRAMECHANNELEXTENSION_H
