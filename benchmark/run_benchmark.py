import os
import sys
import time
import shutil
import platform
import subprocess
import tempfile
import statistics


# ============================================================
# NOVA Benchmark
# ============================================================

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

BUILD_NAME = "nova.exe" if platform.system() == "Windows" else "nova"
COMPILER = os.path.join(ROOT, BUILD_NAME)

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


# ============================================================
# Benchmark Test
# ============================================================

class BenchmarkResult:
    def __init__(self, name):
        self.name = name
        self.times = []
        self.success = False
        self.error = ""

    @property
    def average(self):
        if not self.times:
            return 0.0

        return statistics.mean(self.times)

    @property
    def minimum(self):
        if not self.times:
            return 0.0

        return min(self.times)

    @property
    def maximum(self):
        if not self.times:
            return 0.0

        return max(self.times)

    @property
    def median(self):
        if not self.times:
            return 0.0

        return statistics.median(self.times)


# ============================================================
# NOVA Test Programs
# ============================================================

TESTS = {
    "hello": r'''program Main {
    function main() {
        print("Hello, NOVA!");
    }
}
''',

    "multiple_prints": r'''program Main {
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

    "long_string": r'''program Main {
    function main() {
        print("This is a longer NOVA benchmark string used to test lexer and parser performance.");
    }
}
''',

    "large_program": r'''program Main {
    function main() {
        print("Benchmark 01");
        print("Benchmark 02");
        print("Benchmark 03");
        print("Benchmark 04");
        print("Benchmark 05");
        print("Benchmark 06");
        print("Benchmark 07");
        print("Benchmark 08");
        print("Benchmark 09");
        print("Benchmark 10");
        print("Benchmark 11");
        print("Benchmark 12");
        print("Benchmark 13");
        print("Benchmark 14");
        print("Benchmark 15");
        print("Benchmark 16");
        print("Benchmark 17");
        print("Benchmark 18");
        print("Benchmark 19");
        print("Benchmark 20");
        print("Benchmark 21");
        print("Benchmark 22");
        print("Benchmark 23");
        print("Benchmark 24");
        print("Benchmark 25");
        print("Benchmark 26");
        print("Benchmark 27");
        print("Benchmark 28");
        print("Benchmark 29");
        print("Benchmark 30");
    }
}
'''
}


# ============================================================
# Create temporary NOVA source
# ============================================================

def create_source(directory, name, source):
    path = os.path.join(directory, name + ".nova")

    with open(path, "w", encoding="utf-8") as file:
        file.write(source)

    return path


# ============================================================
# Run one benchmark
# ============================================================

def benchmark_test(name, source):
    result = BenchmarkResult(name)

    with tempfile.TemporaryDirectory(prefix="nova_benchmark_") as directory:

        source_path = create_source(
            directory,
            name,
            source
        )

        # ----------------------------------------------------
        # Warm-up run
        # ----------------------------------------------------

        warmup = run_process(
            [COMPILER, source_path],
            cwd=directory
        )

        if warmup["returncode"] != 0:
            result.success = False
            result.error = warmup["stderr"]

            return result

        # ----------------------------------------------------
        # Actual benchmark runs
        # ----------------------------------------------------

        for _ in range(RUNS):

            output_file = os.path.join(
                directory,
                "nova.out"
            )

            if os.path.exists(output_file):
                os.remove(output_file)

            run = run_process(
                [COMPILER, source_path],
                cwd=directory
            )

            if run["returncode"] != 0:
                result.success = False
                result.error = run["stderr"]

                return result

            result.times.append(run["time"])

        result.success = True

    return result


# ============================================================
# Print header
# ============================================================

def print_header():
    print()
    print("=" * 78)
    print("                         NOVA BENCHMARK")
    print("=" * 78)
    print()

    print(f"Operating System : {platform.system()}")
    print(f"Architecture     : {platform.machine()}")
    print(f"Python           : {platform.python_version()}")
    print(f"Benchmark runs   : {RUNS}")
    print(f"Compiler         : {COMPILER}")

    print()


# ============================================================
# Print table
# ============================================================

def print_table(results):
    print("=" * 78)

    print(
        f"{'Benchmark':<24}"
        f"{'Status':<10}"
        f"{'Average':<14}"
        f"{'Min':<14}"
        f"{'Max':<14}"
    )

    print("-" * 78)

    for result in results:

        status = "PASS" if result.success else "FAIL"

        print(
            f"{result.name:<24}"
            f"{status:<10}"
            f"{format_ms(result.average):<14}"
            f"{format_ms(result.minimum):<14}"
            f"{format_ms(result.maximum):<14}"
        )

    print("=" * 78)


# ============================================================
# Summary
# ============================================================

def print_summary(results, total_time):
    passed = sum(
        1
        for result in results
        if result.success
    )

    failed = len(results) - passed

    all_times = []

    for result in results:
        all_times.extend(result.times)

    print()
    print("Benchmark Summary")
    print("-" * 40)

    print(f"Tests       : {len(results)}")
    print(f"Passed      : {passed}")
    print(f"Failed      : {failed}")

    if all_times:
        print(
            f"Average     : "
            f"{format_ms(statistics.mean(all_times))}"
        )

        print(
            f"Median      : "
            f"{format_ms(statistics.median(all_times))}"
        )

        print(
            f"Fastest     : "
            f"{format_ms(min(all_times))}"
        )

        print(
            f"Slowest     : "
            f"{format_ms(max(all_times))}"
        )

    print(
        f"Total time  : "
        f"{format_ms(total_time)}"
    )

    print("-" * 40)

    if failed == 0:
        print("RESULT: ALL BENCHMARKS PASSED")
    else:
        print("RESULT: SOME BENCHMARKS FAILED")

    print()


# ============================================================
# Main
# ============================================================

def main():
    print_header()

    # --------------------------------------------------------
    # Check compiler
    # --------------------------------------------------------

    if not os.path.isfile(COMPILER):
        print("ERROR")
        print("-" * 40)
        print("NOVA compiler was not found.")
        print()
        print(f"Expected compiler:")
        print(COMPILER)
        print()
        print("Build the compiler before running the benchmark.")
        print()

        return 1

    # --------------------------------------------------------
    # Run benchmarks
    # --------------------------------------------------------

    results = []

    benchmark_start = time.perf_counter()

    for name, source in TESTS.items():

        print(
            f"Running benchmark: {name}...",
            end=" ",
            flush=True
        )

        result = benchmark_test(
            name,
            source
        )

        results.append(result)

        if result.success:
            print(
                f"PASS ({format_ms(result.average)})"
            )
        else:
            print("FAIL")

            if result.error:
                print()
                print(result.error)
                print()

    benchmark_end = time.perf_counter()

    total_time = benchmark_end - benchmark_start

    # --------------------------------------------------------
    # Results
    # --------------------------------------------------------

    print()
    print_table(results)

    print_summary(
        results,
        total_time
    )

    return 0 if all(
        result.success
        for result in results
    ) else 1


# ============================================================
# Entry point
# ============================================================

if __name__ == "__main__":
    sys.exit(main())
