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

add_gtest(planning_rpg_costs                         "planning/heuristics/rpg_costs.cpp")
add_gtest(planning_lmcut                             "planning/heuristics/lmcut.cpp")
target_link_libraries(planning_lmcut PRIVATE Boost::json)

add_gtest(planning_lifted_task                           "planning/lifted_task.cpp")
target_link_libraries(planning_lifted_task PRIVATE Boost::json)
add_gtest(planning_ground_task                           "planning/ground_task.cpp")
target_link_libraries(planning_ground_task PRIVATE Boost::json)
add_gtest(planning_ground_vs_lifted                      "planning/algorithms/ground_vs_lifted.cpp")
target_link_libraries(planning_ground_vs_lifted PRIVATE Boost::json)
add_gtest(planning_brfs_vs_astar                        "planning/algorithms/brfs_vs_astar.cpp")
target_link_libraries(planning_brfs_vs_astar PRIVATE Boost::json)
add_gtest(planning_algorithm_utils                       "planning/algorithms/utils.cpp")
add_gtest(planning_iw                                    "planning/algorithms/iw.cpp")
target_link_libraries(planning_iw PRIVATE Boost::json)
add_gtest(planning_iw_novelty_table                      "planning/algorithms/iw/novelty_table.cpp")
add_gtest(planning_serialized                            "planning/algorithms/serialized.cpp")
target_link_libraries(planning_serialized PRIVATE Boost::json)
add_gtest(planning_siw                                   "planning/algorithms/siw.cpp")
target_link_libraries(planning_siw PRIVATE Boost::json)
