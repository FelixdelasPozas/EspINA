#ifndef PLUGINREGISTER_H
#define PLUGINREGISTER_H

class PluginRegister
{
public:

  static PluginRegister* instance();
  
private:
  PluginRegister(){};
  static PluginRegister* m_instance;
//  QVector<QString, Plugin&> m_plugins;
};

#endif // PLUGINREGISTER_H
