set(GUI_DEPENDECIES
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QUAZIP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${TESTING_DEPENDECIES}
  EspinaCoreTesting
  EspinaExtensionsTesting
  EspinaGUITesting
  EspinaTesting
)

add_subdirectory(ChannelAdapter)
add_subdirectory(ClassificationProxy)
add_subdirectory(ChannelProxy)
add_subdirectory(ClassificationAdapter)
add_subdirectory(ModelAdapter)
add_subdirectory(ModelFactory)
add_subdirectory(SampleAdapter)
# add_subdirectory(View2D)
# add_subdirectory(View3D)
#add_subdirectory(ItemAdapter)