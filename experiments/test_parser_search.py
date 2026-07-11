from experiments.parser_search import add_out_of_memory, add_out_of_time


def test_resource_limit_categories():
    cases = [
        ("", "", 0, 0),
        ("terminate called after throwing std::bad_alloc", "", 1, 0),
        ("", "planner exceeded CPU time limit: 2.0s > 1s", 0, 1),
        ("", "planner exceeded wall-clock time limit: 2.0s > 1s", 0, 1),
    ]

    for run_err, driver_log, out_of_memory, out_of_time in cases:
        props = {}
        add_out_of_memory(run_err, props)
        add_out_of_time(driver_log, props)
        assert props == {"out_of_memory": out_of_memory, "out_of_time": out_of_time}
