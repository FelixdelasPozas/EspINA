# Bound tests
create_test_sourcelist(TEST_SOURCES Scheduler_Tests.cpp # this file is created by this command
  scheduler_simple_task_execution.cpp
  scheduler_simple_task_abort.cpp
  scheduler_simple_task_pause.cpp
  scheduler_pause_non_pauseable_task.cpp
  scheduler_sleep_main_thread.cpp
  scheduler_waiting_tasks.cpp
  scheduler_change_task_priority.cpp
)

include_directories(
  ${CORE_DIR}/MultiTasking
)

set(SUBJECT_SOURCES
  SleepyTask.cpp
)

set(LIB_DEPENDECIES
  ${QT_LIBRARIES}
  ${LIB_DEPENDECIES}
)

add_executable(Scheduler_Tests "" ${TEST_SOURCES} ${SUBJECT_SOURCES} )

target_link_libraries(Scheduler_Tests ${CORE_DEPENDECIES} )

add_test("\"Scheduler: Simple Task Execution\""    Scheduler_Tests scheduler_simple_task_execution)
add_test("\"Scheduler: Simple Task Abort\""        Scheduler_Tests scheduler_simple_task_abort)
add_test("\"Scheduler: Simple Task Pause\""        Scheduler_Tests scheduler_simple_task_pause)
add_test("\"Scheduler: Pause Non Pauseable Task\"" Scheduler_Tests scheduler_pause_non_pauseable_task)
add_test("\"Scheduler: Sleep Main Thread\""        Scheduler_Tests scheduler_sleep_main_thread)
add_test("\"Scheduler: Waiting Tasks\""            Scheduler_Tests scheduler_waiting_tasks)
add_test("\"Scheduler: Change Task Priority\""     Scheduler_Tests scheduler_change_task_priority)