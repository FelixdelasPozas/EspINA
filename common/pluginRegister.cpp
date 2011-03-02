#include "pluginRegister.h"


PluginRegister* PluginRegister::m_instance = 0;

PluginRegister* PluginRegister::instance()
{
  if( !m_instance )
    m_instance = new PluginRegister();
  return m_instance;
}
