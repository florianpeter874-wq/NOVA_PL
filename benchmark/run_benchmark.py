import os
import sys
import time
import platform
import subprocess
import tempfile
import statistics


# ============================================================
# NOVA 1.1 Benchmark Suite
# ============================================================

ROOT = os.path.dirname(
    os.path.dirname(
        os.path.abspath(__file__)
    )
)

if platform.system() == "Windows":
    COMPILER_NAME = "nova.exe"
else:
    COMPILER_NAME = "nova"

COMPILER = os.path.join(
    ROOT,
    COMPILER_NAME
)

RUNS = 10


# ============================================================
# Utility
# ============================================================

def format_ms(seconds):
    return f"{seconds * 1000:.3f} ms"


def run_process(command, cwd=None):
    start = time.perf_counter()

    try:
        process = subprocess.run(
            command,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
    except OSError as error:
        end = time.perf_counter()

        return {
            "returncode": -1,
            "stdout": "",
            "stderr": str(error),
            "time": end - start
        }

    end = time.perf_counter()

    return {
        "returncode": process.returncode,
        "stdout": process.stdout,
        "stderr": process.stderr,
        "time": end - start
    }


def get_output_path(directory):
    if platform.system() == "Windows":
        return os.path.join(
            directory,
            "program.exe"
        )

    return os.path.join(
        directory,
        "program"
    )


# ============================================================
# Benchmark Result
# ============================================================

class BenchmarkResult:

    def __init__(self, name):
        self.name = name

        self.compile_times = []
        self.runtime_times = []

        self.success = False

        self.error_type = ""
        self.error = ""

        self.expected_output = ""
        self.actual_output = ""

    @property
    def compile_average(self):
        if not self.compile_times:
            return 0.0

        return statistics.mean(self.compile_times)

    @property
    def compile_minimum(self):
        if not self.compile_times:
            return 0.0

        return min(self.compile_times)

    @property
    def compile_maximum(self):
        if not self.compile_times:
            return 0.0

        return max(self.compile_times)

    @property
    def compile_median(self):
        if not self.compile_times:
            return 0.0

        return statistics.median(self.compile_times)

    @property
    def runtime_average(self):
        if not self.runtime_times:
            return 0.0

        return statistics.mean(self.runtime_times)

    @property
    def runtime_minimum(self):
        if not self.runtime_times:
            return 0.0

        return min(self.runtime_times)

    @property
    def runtime_maximum(self):
        if not self.runtime_times:
            return 0.0

        return max(self.runtime_times)

    @property
    def runtime_median(self):
        if not self.runtime_times:
            return 0.0

        return statistics.median(self.runtime_times)


# ============================================================
# NOVA Test Programs
# ============================================================

TESTS = [
    {
        "name": "hello",

        "source": r'''entryp Main main

program Main {
    function main() {
        print("Hello, NOVA!");
    }
}
''',

        "expected": "Hello, NOVA!\n"
    },

    {
        "name": "multiple_prints",

        "source": r'''entryp Main main

program Main {
    function main() {
        print("One");
        print("Two");
        print("Three");
        print("Four");
        print("Five");
        print("Six");
        print("Seven");
        print("Eight");
        print("Nine");
        print("Ten");
    }
}
''',

        "expected": (
            "One\n"
            "Two\n"
            "Three\n"
            "Four\n"
            "Five\n"
            "Six\n"
            "Seven\n"
            "Eight\n"
            "Nine\n"
            "Ten\n"
        )
    },

    {
        "name": "long_string",

        "source": r'''entryp Main main

program Main {
    function main() {
        print("This is a longer NOVA benchmark string used to test lexer and parser performance.");
    }
}
''',

        "expected": (
            "This is a longer NOVA benchmark string used "
            "to test lexer and parser performance.\n"
        )
    },

    {
        "name": "variable",

        "source": r'''entryp Main main

program Main {
    function main() {
        set int x = 42;
        print(x);
    }
}
''',

        "expected": "42\n"
    },

    {
        "name": "integer_expression",

        "source": r'''entryp Main main

program Main {
    function main() {
        set int x = 10;
        print(x + 32);
    }
}
''',

        "expected": "42\n"
    },

    {
        "name": "string_integer",

        "source": r'''entryp Main main

program Main {
    function main() {
        set int x = 42;
        print("The answer is:" + x);
    }
}
''',

        "expected": "The answer is:42\n"
    },

    {
        "name": "function_call",

        "source": r'''entryp Main main

program Main {
    function hello() {
        print("Hello from function!");
    }

    function main() {
        hello();
    }
}
''',

        "expected": "Hello from function!\n"
    },

    {
        "name": "multiple_function_calls",

        "source": r'''entryp Main main

program Main {
    function first() {
        print("FIRST");
    }

    function second() {
        print("SECOND");
    }

    function third() {
        print("THIRD");
    }

    function main() {
        first();
        second();
        third();
    }
}
''',

        "expected": (
            "FIRST\n"
            "SECOND\n"
            "THIRD\n"
        )
    },

    {
        "name": "if_true",

        "source": r'''entryp Main main

program Main {
    function main() {
        set int x = 42;

        if (x == 42) {
            print("YES!");
        } else {
            print("NO!");
        }
    }
}
''',

        "expected": "YES!\n"
    },

    {
        "name": "if_else",

        "source": r'''entryp Main main

program Main {
    function main() {
        set int x = 10;

        if (x == 42) {
            print("WRONG");
        } else {
            print("CORRECT");
        }
    }
}
''',

        "expected": "CORRECT\n"
    },

    {
        "name": "comparisons",

        "source": r'''entryp Main main

program Main {
    function main() {
        set int x = 42;

        if (x != 10) {
            print("NOT_EQUAL");
        }

        if (x > 10) {
            print("GREATER");
        }

        if (x >= 42) {
            print("GREATER_EQUAL");
        }

        if (x < 100) {
            print("LESS");
        }

        if (x <= 42) {
            print("LESS_EQUAL");
        }
    }
}
''',

        "expected": (
            "NOT_EQUAL\n"
            "GREATER\n"
            "GREATER_EQUAL\n"
            "LESS\n"
            "LESS_EQUAL\n"
        )
    },

    {
        "name": "nested_logic",

        "source": r'''entryp Main main

program Main {
    function check() {
        set int x = 42;

        if (x == 42) {
            print("CHECK PASSED");

            if (x > 40) {
                print("VALUE VALID");
            }
        }
    }

    function main() {
        check();
    }
}
''',

        "expected": (
            "CHECK PASSED\n"
            "VALUE VALID\n"
        )
    },

    {
        "name": "large_program",

        "source": r'''entryp Main main

program Main {

    function first() {
        print("Benchmark 01");
        print("Benchmark 02");
        print("Benchmark 03");
        print("Benchmark 04");
        print("Benchmark 05");
    }

    function second() {
        print("Benchmark 06");
        print("Benchmark 07");
        print("Benchmark 08");
        print("Benchmark 09");
        print("Benchmark 10");
    }

    function third() {
        print("Benchmark 11");
        print("Benchmark 12");
        print("Benchmark 13");
        print("Benchmark 14");
        print("Benchmark 15");
    }

    function fourth() {
        print("Benchmark 16");
        print("Benchmark 17");
        print("Benchmark 18");
        print("Benchmark 19");
        print("Benchmark 20");
    }

    function main() {
        first();
        second();
        third();
        fourth();

        set int x = 42;

        print(x);
        print("Value:" + x);

        if (x == 42) {
            print("Benchmark condition passed!");
        } else {
            print("Benchmark condition failed!");
        }
    }
}
''',

        "expected": (
            "Benchmark 01\n"
            "Benchmark 02\n"
            "Benchmark 03\n"
            "Benchmark 04\n"
            "Benchmark 05\n"
            "Benchmark 06\n"
            "Benchmark 07\n"
            "Benchmark 08\n"
            "Benchmark 09\n"
            "Benchmark 10\n"
            "Benchmark 11\n"
            "Benchmark 12\n"
            "Benchmark 13\n"
            "Benchmark 14\n"
            "Benchmark 15\n"
            "Benchmark 16\n"
            "Benchmark 17\n"
            "Benchmark 18\n"
            "Benchmark 19\n"
            "Benchmark 20\n"
            "42\n"
            "Value:42\n"
            "Benchmark condition passed!\n"
        )
    }
]


# ============================================================
# Source creation
# ============================================================

def create_source(directory, name, source):
    path = os.path.join(
        directory,
        name + ".nova"
    )

    with open(
        path,
        "w",
        encoding="utf-8"
    ) as file:
        file.write(source)

    return path


# ============================================================
# Compile NOVA source
# ============================================================

def compile_program(
    source_path,
    output_path,
    directory
):
    return run_process(
        [
            COMPILER,
            source_path,
            "-o",
            output_path
        ],
        cwd=directory
    )


# ============================================================
# Run compiled program
# ============================================================

def run_program(
    output_path,
    directory
):
    return run_process(
        [
            output_path
        ],
        cwd=directory
    )


# ============================================================
# Benchmark one test
# ============================================================

def benchmark_test(test):
    result = BenchmarkResult(
        test["name"]
    )

    result.expected_output = test["expected"]

    with tempfile.TemporaryDirectory(
        prefix="nova_benchmark_"
    ) as directory:

        source_path = create_source(
            directory,
            test["name"],
            test["source"]
        )

        output_path = get_output_path(
            directory
        )

        # ----------------------------------------------------
        # Warm-up compilation
        # ----------------------------------------------------

        warmup_compile = compile_program(
            source_path,
            output_path,
            directory
        )

        if warmup_compile["returncode"] != 0:
            result.error_type = "COMPILATION"
            result.error = (
                warmup_compile["stderr"]
                or warmup_compile["stdout"]
            )

            return result

        # ----------------------------------------------------
        # Warm-up execution
        # ----------------------------------------------------

        warmup_run = run_program(
            output_path,
            directory
        )

        if warmup_run["returncode"] != 0:
            result.error_type = "RUNTIME"
            result.error = (
                warmup_run["stderr"]
                or warmup_run["stdout"]
            )

            return result

        if warmup_run["stdout"] != test["expected"]:
            result.error_type = "OUTPUT"
            result.error = (
                "Program output does not match."
            )

            result.actual_output = (
                warmup_run["stdout"]
            )

            return result

        # ----------------------------------------------------
        # Benchmark runs
        # ----------------------------------------------------

        for _ in range(RUNS):

            if os.path.exists(output_path):
                try:
                    os.remove(output_path)
                except OSError:
                    pass

            # ------------------------------------------------
            # Compile
            # ------------------------------------------------

            compile_result = compile_program(
                source_path,
                output_path,
                directory
            )

            if compile_result["returncode"] != 0:
                result.error_type = "COMPILATION"
                result.error = (
                    compile_result["stderr"]
                    or compile_result["stdout"]
                )

                return result

            result.compile_times.append(
                compile_result["time"]
            )

            # ------------------------------------------------
            # Execute
            # ------------------------------------------------

            run_result = run_program(
                output_path,
                directory
            )

            if run_result["returncode"] != 0:
                result.error_type = "RUNTIME"
                result.error = (
                    run_result["stderr"]
                    or run_result["stdout"]
                )

                return result

            if run_result["stdout"] != test["expected"]:
                result.error_type = "OUTPUT"
                result.error = (
                    "Program output does not match."
                )

                result.actual_output = (
                    run_result["stdout"]
                )

                return result

            result.runtime_times.append(
                run_result["time"]
            )

        result.success = True

    return result


# ============================================================
# Header
# ============================================================

def print_header():
    print()
    print("=" * 92)
    print("                              NOVA BENCHMARK")
    print("=" * 92)
    print()

    print("Version          : NOVA 1.1")
    print(f"Operating System : {platform.system()}")
    print(f"Architecture     : {platform.machine()}")
    print(f"Python           : {platform.python_version()}")
    print(f"Runs per test    : {RUNS}")
    print(f"Compiler         : {COMPILER}")

    print()


# ============================================================
# Results table
# ============================================================

def print_table(results):
    print("=" * 92)

    print(
        f"{'Benchmark':<28}"
        f"{'Status':<10}"
        f"{'Compile':<17}"
        f"{'Runtime':<17}"
        f"{'Total':<17}"
    )

    print("-" * 92)

    for result in results:
        status = (
            "PASS"
            if result.success
            else "FAIL"
        )

        total = (
            result.compile_average
            + result.runtime_average
        )

        print(
            f"{result.name:<28}"
            f"{status:<10}"
            f"{format_ms(result.compile_average):<17}"
            f"{format_ms(result.runtime_average):<17}"
            f"{format_ms(total):<17}"
        )

    print("=" * 92)


# ============================================================
# Detailed statistics
# ============================================================

def print_details(results):
    print()
    print("Detailed Statistics")
    print("=" * 60)

    for result in results:
        print()
        print(result.name)
        print("-" * 60)

        if not result.success:
            print("Status : FAIL")
            print(f"Type   : {result.error_type}")

            if result.error:
                print()
                print(result.error)

            if result.error_type == "OUTPUT":
                print()
                print("Expected output:")
                print(result.expected_output)

                print("Actual output:")
                print(result.actual_output)

            continue

        print("Status          : PASS")

        print(
            f"Compile average : "
            f"{format_ms(result.compile_average)}"
        )

        print(
            f"Compile median  : "
            f"{format_ms(result.compile_median)}"
        )

        print(
            f"Compile minimum : "
            f"{format_ms(result.compile_minimum)}"
        )

        print(
            f"Compile maximum : "
            f"{format_ms(result.compile_maximum)}"
        )

        print(
            f"Runtime average : "
            f"{format_ms(result.runtime_average)}"
        )

        print(
            f"Runtime median  : "
            f"{format_ms(result.runtime_median)}"
        )

        print(
            f"Runtime minimum : "
            f"{format_ms(result.runtime_minimum)}"
        )

        print(
            f"Runtime maximum : "
            f"{format_ms(result.runtime_maximum)}"
        )


# ============================================================
# Summary
# ============================================================

def print_summary(
    results,
    total_time
):
    passed = sum(
        result.success
        for result in results
    )

    failed = (
        len(results)
        - passed
    )

    compile_times = []
    runtime_times = []

    for result in results:
        compile_times.extend(
            result.compile_times
        )

        runtime_times.extend(
            result.runtime_times
        )

    print()
    print("=" * 60)
    print("Benchmark Summary")
    print("=" * 60)

    print(
        f"Tests          : {len(results)}"
    )

    print(
        f"Passed         : {passed}"
    )

    print(
        f"Failed         : {failed}"
    )

    if compile_times:
        print()

        print(
            f"Compile avg    : "
            f"{format_ms(statistics.mean(compile_times))}"
        )

        print(
            f"Compile median : "
            f"{format_ms(statistics.median(compile_times))}"
        )

        print(
            f"Compile fastest: "
            f"{format_ms(min(compile_times))}"
        )

        print(
            f"Compile slowest: "
            f"{format_ms(max(compile_times))}"
        )

    if runtime_times:
        print()

        print(
            f"Runtime avg    : "
            f"{format_ms(statistics.mean(runtime_times))}"
        )

        print(
            f"Runtime median : "
            f"{format_ms(statistics.median(runtime_times))}"
        )

        print(
            f"Runtime fastest : "
            f"{format_ms(min(runtime_times))}"
        )

        print(
            f"Runtime slowest : "
            f"{format_ms(max(runtime_times))}"
        )

    print()

    print(
        f"Total benchmark: "
        f"{format_ms(total_time)}"
    )

    print("=" * 60)

    if failed == 0:
        print()
        print(
            "RESULT: ALL NOVA BENCHMARKS PASSED"
        )
    else:
        print()
        print(
            "RESULT: NOVA BENCHMARK FAILED"
        )

    print()


# ============================================================
# Main
# ============================================================

def main():
    print_header()

    # --------------------------------------------------------
    # Compiler check
    # --------------------------------------------------------

    if not os.path.isfile(COMPILER):
        print("ERROR")
        print("-" * 60)

        print(
            "NOVA compiler was not found."
        )

        print()

        print("Expected:")
        print(COMPILER)

        print()

        print(
            "Build nova.exe before running "
            "the benchmark."
        )

        print()

        return 1

    # --------------------------------------------------------
    # Run benchmarks
    # --------------------------------------------------------

    results = []

    benchmark_start = time.perf_counter()

    for test in TESTS:
        name = test["name"]

        print(
            f"Running {name:<28}",
            end="",
            flush=True
        )

        result = benchmark_test(test)

        results.append(result)

        if result.success:
            print(
                " PASS  "
                f"[compile "
                f"{format_ms(result.compile_average)}, "
                f"runtime "
                f"{format_ms(result.runtime_average)}]"
            )
        else:
            print(" FAIL")

            print(
                f"       {result.error_type}: "
                f"{result.error}"
            )

    benchmark_end = time.perf_counter()

    total_time = (
        benchmark_end
        - benchmark_start
    )

    # --------------------------------------------------------
    # Results
    # --------------------------------------------------------

    print()

    print_table(results)

    print_details(results)

    print_summary(
        results,
        total_time
    )

    if all(
        result.success
        for result in results
    ):
        return 0

    return 1


# ============================================================
# Entry point
# ============================================================

if __name__ == "__main__":
    sys.exit(main())
