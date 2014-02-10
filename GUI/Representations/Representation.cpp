/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Representation.h"

#include <GUI/View/View2D.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
Representation::Representation(RenderView *view) 
: m_brightness(0)
, m_contrast(1)
, m_opacity(1)
, m_color(Qt::white)
, m_highlight(false)
, m_view(view)
, m_lastUpdatedTime(0)
, m_active(true)
, m_visible(true)
{}

//-----------------------------------------------------------------------------
void Representation::setActive(bool value, RenderView *view)
{
  if (!view || view == m_view)
  {
    m_active = value;

    updateVisibility(isVisible());
  }

  for(auto clone: m_clones)
    clone->setActive(value, view);
}

//-----------------------------------------------------------------------------
QString Representation::serializeSettings()
{
  return isActive()?"On":"Off"; 
}

//-----------------------------------------------------------------------------
void Representation::restoreSettings(QString settings)
{
  if (settings == "On")
    setActive(true);
  else if (settings == "Off")
    setActive(false);
}

//-----------------------------------------------------------------------------
void Representation::setVisible(bool visible)
{
  m_visible = visible;

  updateVisibility(isVisible());
}

//-----------------------------------------------------------------------------
RepresentationSPtr Representation::clone(View2D *view)
{
  RepresentationSPtr representation = cloneImplementation(view);

  if (representation)
  {
    representation->setActive(isActive());

    m_clones << representation;
  }

  return representation;
}

//-----------------------------------------------------------------------------
RepresentationSPtr Representation::clone(View3D *view)
{
  RepresentationSPtr representation = cloneImplementation(view);

  if (representation)
    m_clones << representation;

  return representation;
}
