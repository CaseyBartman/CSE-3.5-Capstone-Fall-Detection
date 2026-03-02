#!/usr/bin/env python3
"""
Test Result Parser for Fall Detection Test Suite

Parses Google Test output and displays results in a clean, organized format.
Shows passed tests, failed tests with their errors, and detailed summaries.
"""

import subprocess
import sys
import re
from pathlib import Path
from typing import List, Dict, Tuple


class Colors:
    """ANSI color codes for terminal output"""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    DIM = '\033[2m'
    
    # Foreground colors
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'
    
    # Background colors
    BG_RED = '\033[41m'
    BG_GREEN = '\033[42m'
    BG_YELLOW = '\033[43m'


class TestParser:
    """Parses Google Test output"""
    
    def __init__(self, output: str):
        self.output = output
        self.lines = output.split('\n')
        self.passed_tests: List[Dict] = []
        self.failed_tests: List[Dict] = []
        self.test_errors: Dict[str, List[str]] = {}
        self.summary = {}
        
    def parse(self) -> None:
        """Parse the test output"""
        self._extract_tests()
        self._extract_summary()
        self._extract_errors()
        
    def _extract_tests(self) -> None:
        """Extract passed and failed test names"""
        seen_passed = set()
        seen_failed = set()
        
        for line in self.lines:
            # Match passed tests: [ RUN      ] and [       OK ]
            if '[       OK ]' in line:
                match = re.search(r'\[       OK \] (\S+) \((\d+) ms\)', line)
                if match:
                    test_name = match.group(1)
                    if test_name not in seen_passed:
                        duration = int(match.group(2))
                        self.passed_tests.append({'name': test_name, 'duration': duration})
                        seen_passed.add(test_name)
            
            # Match failed tests: [ RUN      ] and [  FAILED  ]
            elif '[  FAILED  ]' in line and 'listed below' not in line:
                match = re.search(r'\[  FAILED  \] (\S+)', line)
                if match:
                    test_name = match.group(1)
                    # Ensure test name is valid (not "N tests," pattern)
                    if '.' in test_name and test_name not in seen_failed:
                        self.failed_tests.append({'name': test_name})
                        seen_failed.add(test_name)
    
    def _extract_summary(self) -> None:
        """Extract test summary (passed/failed counts, total time)"""
        for line in self.lines:
            # Total tests and suites
            match = re.search(r'\[==========\] (\d+) tests? from (\d+) test suite', line)
            if match:
                self.summary['total_tests'] = int(match.group(1))
                self.summary['total_suites'] = int(match.group(2))
            
            # Passed tests
            match = re.search(r'\[  PASSED  \] (\d+) tests?\.?', line)
            if match:
                self.summary['passed'] = int(match.group(1))
            
            # Failed tests
            match = re.search(r'\[  FAILED  \] (\d+) tests?', line)
            if match:
                self.summary['failed'] = int(match.group(1))
            
            # Duration
            match = re.search(r'\((\d+) ms total\)', line)
            if match:
                self.summary['duration_ms'] = int(match.group(1))
    
    def _extract_errors(self) -> None:
        """Extract error messages for failed tests"""
        current_test = None
        error_lines = []
        in_error_section = False
        
        for i, line in enumerate(self.lines):
            # Start of a failed test details
            if '[ RUN      ]' in line and any(t['name'] in line for t in self.failed_tests):
                current_test = None
                # Extract test name
                match = re.search(r'\[ RUN      \] (\S+)', line)
                if match:
                    current_test = match.group(1)
                    error_lines = []
                    in_error_section = True
            
            # End of test (next test starts or summary begins)
            elif in_error_section and (
                '[ RUN      ]' in line or 
                '[----------]' in line or 
                '[==========]' in line
            ):
                if current_test and error_lines:
                    self.test_errors[current_test] = error_lines
                in_error_section = False
                current_test = None
                error_lines = []
            
            # Collect error content
            elif in_error_section and current_test:
                # Skip gtest-specific lines
                if not any(x in line for x in ['[ RUN', '[OK]', '[FAILED]', '[---', '[===', 'Global test']):
                    if line.strip():  # Non-empty line
                        error_lines.append(line)
        
        # Don't forget the last test
        if current_test and error_lines:
            self.test_errors[current_test] = error_lines


class TestReporter:
    """Formats and displays test results"""
    
    def __init__(self, parser: TestParser):
        self.parser = parser
    
    def print_header(self) -> None:
        """Print test run header"""
        print(f"\n{Colors.CYAN}{'═' * 70}{Colors.RESET}")
        print(f"{Colors.GREEN}{Colors.BOLD}Fall Detection System - Test Results{Colors.RESET}")
        print(f"{Colors.CYAN}{'═' * 70}{Colors.RESET}\n")
    
    def print_summary(self) -> None:
        """Print test summary statistics"""
        summary = self.parser.summary
        
        if not summary:
            return
        
        total = summary.get('total_tests', 0)
        passed = summary.get('passed', 0)
        failed = summary.get('failed', 0)
        duration = summary.get('duration_ms', 0)
        
        status_icon = f"{Colors.GREEN}✓{Colors.RESET}" if failed == 0 else f"{Colors.RED}✗{Colors.RESET}"
        
        print(f"{Colors.BOLD}Test Summary:{Colors.RESET}")
        print(f"  {status_icon} Total Tests:  {total}")
        print(f"  {Colors.GREEN}✓ Passed:      {passed}{Colors.RESET}")
        
        if failed > 0:
            print(f"  {Colors.RED}✗ Failed:      {failed}{Colors.RESET}")
        
        print(f"  ⏱ Duration:    {duration}ms")
        print()
    
    def print_passed_tests(self) -> None:
        """Print list of passed tests"""
        if not self.parser.passed_tests:
            return
        
        print(f"{Colors.BOLD}{Colors.GREEN}✓ PASSED TESTS ({len(self.parser.passed_tests)}):{Colors.RESET}")
        
        # Group by suite to avoid repeating suite names
        current_suite = None
        for test in self.parser.passed_tests:
            suite_name = test['name'].split('.')[0]
            test_name = test['name'].split('.')[1] if '.' in test['name'] else test['name']
            duration = test['duration']
            
            # Print suite header only if it changed
            if suite_name != current_suite:
                if current_suite is not None:
                    print()  # Blank line between suites
                print(f"  {Colors.GREEN}✓{Colors.RESET} {suite_name}")
                current_suite = suite_name
            
            print(f"      └─ {test_name} ({duration}ms)")
        print()
    
    def print_failed_tests(self) -> None:
        """Print detailed information about failed tests"""
        if not self.parser.failed_tests:
            return
        
        print(f"{Colors.BOLD}{Colors.RED}✗ FAILED TESTS ({len(self.parser.failed_tests)}):{Colors.RESET}\n")
        
        for i, test in enumerate(self.parser.failed_tests, 1):
            test_name = test['name']
            suite_name = test_name.split('.')[0]
            test_method = test_name.split('.')[1]
            
            print(f"{Colors.RED}{Colors.BOLD}[{i}] {test_name}{Colors.RESET}")
            print(f"    Suite:   {suite_name}")
            print(f"    Method:  {test_method}")
            
            # Print error details if available
            if test_name in self.parser.test_errors:
                errors = self.parser.test_errors[test_name]
                if errors:
                    print(f"{Colors.DIM}    Error Details:{Colors.RESET}")
                    for error_line in errors[:10]:  # Limit to first 10 lines
                        print(f"      {Colors.YELLOW}{error_line}{Colors.RESET}")
                    if len(errors) > 10:
                        print(f"      ... and {len(errors) - 10} more lines")
            print()
    
    def print_report(self) -> int:
        """Print complete test report and return exit code"""
        self.print_header()
        self.print_summary()
        self.print_passed_tests()
        self.print_failed_tests()
        
        # Print footer
        failed = self.parser.summary.get('failed', 0)
        if failed == 0:
            print(f"{Colors.CYAN}{'═' * 70}{Colors.RESET}")
            print(f"{Colors.GREEN}{Colors.BOLD}All Tests Passed! ✓{Colors.RESET}")
            print(f"{Colors.CYAN}{'═' * 70}{Colors.RESET}\n")
            return 0
        else:
            print(f"{Colors.CYAN}{'═' * 70}{Colors.RESET}")
            print(f"{Colors.RED}{Colors.BOLD}{failed} Test(s) Failed ✗{Colors.RESET}")
            print(f"{Colors.CYAN}{'═' * 70}{Colors.RESET}\n")
            return 1


def run_tests(test_binary: str) -> Tuple[str, int]:
    """Run the test binary and capture output"""
    try:
        result = subprocess.run(
            [test_binary],
            capture_output=True,
            text=True,
            timeout=60
        )
        return result.stdout + result.stderr, result.returncode
    except FileNotFoundError:
        print(f"{Colors.RED}ERROR: Test binary not found: {test_binary}{Colors.RESET}")
        sys.exit(1)
    except subprocess.TimeoutExpired:
        print(f"{Colors.RED}ERROR: Test execution timed out{Colors.RESET}")
        sys.exit(1)


def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python parse_tests.py <test_binary_path>")
        sys.exit(1)
    
    test_binary = sys.argv[1]
    
    # Run tests and capture output
    output, exit_code = run_tests(test_binary)
    
    # Parse results
    parser = TestParser(output)
    parser.parse()
    
    # Display report
    reporter = TestReporter(parser)
    report_exit_code = reporter.print_report()
    
    # Return appropriate exit code
    sys.exit(report_exit_code)


if __name__ == '__main__':
    main()
