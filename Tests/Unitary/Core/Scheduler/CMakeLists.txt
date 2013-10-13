# Bound tests
create_test_sourcelist(TEST_SOURCES Scheduler_Tests.cpp # this file is created by this command
  scheduler_simple_task_execution.cpp
  scheduler_simple_task_abort.cpp
  scheduler_simple_task_pause.cpp
  scheduler_sleep_main_thread.cpp
  scheduler_waiting_tasks.cpp
  scheduler_change_task_priority.cpp
)

include_directories(
  ${CORE_DIR}/MultiTasking
)
  
qt4_wrap_cpp(SUBJECT_MOCS
  ${CORE_DIR}/MultiTasking/Scheduler.h
  ${CORE_DIR}/MultiTasking/Task.h
)

set(SUBJECT_SOURCES
  ${CORE_DIR}/MultiTasking/Scheduler.cpp
  ${CORE_DIR}/MultiTasking/Task.cpp
  SleepyTask.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Scheduler_Tests
  ${TEST_SOURCES}
  ${SUBJECT_MOCS}
  ${SUBJECT_SOURCES}
  )
  
target_link_libraries(Scheduler_Tests ${LIB_DEPENDECIES} )

add_test("\"Scheduler: Simple Task Execution\""  Scheduler_Tests scheduler_simple_task_execution)
add_test("\"Scheduler: Simple Task Abort\""      Scheduler_Tests scheduler_simple_task_abort)
add_test("\"Scheduler: Simple Task Pause\""      Scheduler_Tests scheduler_simple_task_pause)
add_test("\"Scheduler: Sleep Main Thread\""      Scheduler_Tests scheduler_sleep_main_thread)
add_test("\"Scheduler: Waiting Tasks\""          Scheduler_Tests scheduler_waiting_tasks)
add_test("\"Scheduler: Change Task Priority\""   Scheduler_Tests scheduler_change_task_priority)