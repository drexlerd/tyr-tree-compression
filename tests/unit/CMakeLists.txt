enable_testing()

find_package(GTest "1.11.0" REQUIRED COMPONENTS GTest PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(Python 3.8 QUIET COMPONENTS Interpreter Development.Module)
if(Python_FOUND)
    find_package(nanobind CONFIG QUIET PATHS ${CMAKE_PREFIX_PATH})
endif()

# Helper function to create and register a test executable
function(add_gtest test_name source_file)
    add_executable(${test_name} ${source_file} main.cpp)
    target_link_libraries(${test_name} PRIVATE tyr::core TBB::tbb GTest::gtest)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add each test source file as a separate test executable



add_gtest(formalism_planning_invariants_synthesis        "formalism/planning/invariants/synthesis.cpp")
target_link_libraries(formalism_planning_invariants_synthesis PRIVATE Boost::json)
add_gtest(formalism_planning_parser_symbol_index_stability "formalism/planning/parser_symbol_index_stability.cpp")
target_link_libraries(formalism_planning_parser_symbol_index_stability PRIVATE Boost::json)
add_gtest(formalism_builder                              "formalism/builder.cpp")
add_gtest(formalism_repository                           "formalism/repository.cpp")
add_gtest(formalism_view                                 "formalism/view.cpp")
add_gtest(formalism_index                                "formalism/index.cpp")
add_gtest(formalism_unification                          "formalism/unification.cpp")

add_gtest(datalog_cost_policy                           "datalog/cost_policy.cpp")
add_gtest(datalog_ground_queue                         "datalog/ground_queue.cpp")

add_gtest(planning_heuristics_cross_rpg                  "planning/heuristics/cross/rpg.cpp")
target_link_libraries(planning_heuristics_cross_rpg PRIVATE Boost::json)
foreach(task_kind ground lifted)
    foreach(heuristic rpg_max rpg_add rpg_ff lmcut)
        add_gtest(planning_heuristics_${task_kind}_${heuristic} "planning/heuristics/${task_kind}/${heuristic}.cpp")
        target_link_libraries(planning_heuristics_${task_kind}_${heuristic} PRIVATE Boost::json)
    endforeach()
endforeach()

add_gtest(planning_algorithms_statistics_cross_brfs_vs_astar "planning/algorithms/statistics/cross/brfs_vs_astar.cpp")
target_link_libraries(planning_algorithms_statistics_cross_brfs_vs_astar PRIVATE Boost::json)
foreach(task_kind ground lifted)
    foreach(algorithm brfs astar gbfs iw siw)
        add_gtest(planning_algorithms_statistics_${task_kind}_${algorithm} "planning/algorithms/statistics/${task_kind}/${algorithm}.cpp")
        target_link_libraries(planning_algorithms_statistics_${task_kind}_${algorithm} PRIVATE Boost::json)
    endforeach()
endforeach()
add_gtest(planning_algorithms_utils                      "planning/algorithms/utils.cpp")
add_gtest(planning_algorithms_portable_shuffle           "planning/algorithms/portable_shuffle.cpp")
add_gtest(planning_algorithms_iw_novelty_table           "planning/algorithms/iw/novelty_table.cpp")
add_gtest(planning_algorithms_serialized                 "planning/algorithms/serialized.cpp")
target_link_libraries(planning_algorithms_serialized PRIVATE Boost::json)
add_gtest(planning_lifted_state_storage                  "planning/lifted/state_storage.cpp")
