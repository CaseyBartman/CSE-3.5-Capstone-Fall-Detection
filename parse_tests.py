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
        in_test_section = False
        
        for i, line in enumerate(self.lines):
            # Start of a test - check if it's one that failed
            run_match = re.search(r'\[\s*RUN\s+\]\s+(\S+)', line)
            if run_match:
                test_name = run_match.group(1)
                # Check if this test is in our failed tests list
                if any(t['name'] == test_name for t in self.failed_tests):
                    current_test = test_name
                    error_lines = []
                    in_test_section = True
                else:
                    in_test_section = False
                    current_test = None
            
            # End of test section (when we hit FAILED or OK marker)
            elif in_test_section and re.search(r'\[\s*(FAILED|OK)\s*\]', line):
                if current_test and error_lines:
                    self.test_errors[current_test] = error_lines
                in_test_section = False
                current_test = None
                error_lines = []
            
            # Collect error content (lines between RUN and FAILED/OK)
            elif in_test_section and current_test:
                # Skip separator lines and empty lines
                if not re.search(r'^\s*\[-+\]|^\s*\[=+\]', line) and line.strip():
                    error_lines.append(line)
        
        # Don't forget the last test
        if current_test and error_lines:
            self.test_errors[current_test] = error_lines


class TestReporter:
    """Formats and displays test results"""
    
    # Map test names to their logical categories
    TEST_CATEGORIES = {
        'Initialization': [
            'InitializationCreatesAllComponents',
            'SystemStartsInIdleState',
            'AllMocksAreInitialized',
        ],
        'State Transitions': [
            'IdleToPollingTransition',
            'PollingToAlarmOnFallDetection',
            'AlarmToPolling_OnButtonPress',
            'PollingToInputPaused_OnShortPress',
            'PollingToCalibration_OnLongPress',
            'InputPausedToPolling_OnTimeout',
            'CalibrationToPolling_OnTimeout',
        ],
        'Fall Detection Logic': [
            'FallDetectedWhenPressureRisesAboveThreshold',
            'AlarmOnGradualPressureChange',
            'AlarmTriggerCountIncrementsCorrectly',
            'AlarmClearedOnButtonPress',
        ],
        'Button Handling': [
            'ShortPressClearsAlarm',
            'ShortPressPausesMonitoring',
            'LongPressTriggerCalibration',
            'MultipleButtonPressesHandleSequentially',
        ],
        'Timing & Expiration': [
            'PauseDurationExpires_AtExactTime',
            'CalibrationDurationExpires_AtExactTime',
            'PauseDoesNotExpireEarly',
            'CalibrationDoesNotExpireEarly',
        ],
        'Integration & Edge Cases': [
            'StateTransitionSequence',
            'ResetBetweenTests',
            'AlertCountersResetOnNewDetector',
        ],
    }
    
    def __init__(self, parser: TestParser):
        self.parser = parser
    
    def get_test_category(self, test_name: str) -> str:
        """Get the category for a test based on its name"""
        test_method = test_name.split('.')[1] if '.' in test_name else test_name
        for category, tests in self.TEST_CATEGORIES.items():
            if test_method in tests:
                return category
        return 'Uncategorized'
    
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
        """Print list of passed tests organized by category"""
        if not self.parser.passed_tests:
            return
        
        print(f"{Colors.BOLD}{Colors.GREEN}✓ PASSED TESTS ({len(self.parser.passed_tests)}):{Colors.RESET}\n")
        
        # Group tests by category
        tests_by_category = {}
        for test in self.parser.passed_tests:
            category = self.get_test_category(test['name'])
            if category not in tests_by_category:
                tests_by_category[category] = []
            tests_by_category[category].append(test)
        
        # Print each category with its tests
        for category in self.TEST_CATEGORIES.keys():
            if category not in tests_by_category:
                continue
            
            tests = tests_by_category[category]
            print(f"  {Colors.GREEN}✓{Colors.RESET} {Colors.BOLD}{category}{Colors.RESET} ({len(tests)} test{'s' if len(tests) != 1 else ''})")
            
            for test in tests:
                test_name = test['name'].split('.')[1] if '.' in test['name'] else test['name']
                duration = test['duration']
                print(f"      └─ {test_name} ({duration}ms)")
            
            print()  # Blank line between categories
        
        # Print uncategorized tests if any
        if 'Uncategorized' in tests_by_category:
            tests = tests_by_category['Uncategorized']
            print(f"  {Colors.YELLOW}?{Colors.RESET} {Colors.BOLD}Uncategorized{Colors.RESET} ({len(tests)} test{'s' if len(tests) != 1 else ''})")
            
            for test in tests:
                test_name = test['name'].split('.')[1] if '.' in test['name'] else test['name']
                duration = test['duration']
                print(f"      └─ {test_name} ({duration}ms)")
            
            print()
        
        print()
    
    def print_failed_tests(self) -> None:
        """Print detailed information about failed tests organized by category"""
        if not self.parser.failed_tests:
            return
        
        print(f"{Colors.BOLD}{Colors.RED}✗ FAILED TESTS ({len(self.parser.failed_tests)}):{Colors.RESET}\n")
        
        # Group tests by category
        tests_by_category = {}
        for test in self.parser.failed_tests:
            category = self.get_test_category(test['name'])
            if category not in tests_by_category:
                tests_by_category[category] = []
            tests_by_category[category].append(test)
        
        # Print each category with its failed tests
        test_counter = 1
        for category in self.TEST_CATEGORIES.keys():
            if category not in tests_by_category:
                continue
            
            tests = tests_by_category[category]
            print(f"  {Colors.RED}{Colors.BOLD}{category}{Colors.RESET} ({len(tests)} failed):")
            
            for test in tests:
                test_name = test['name']
                suite_name = test_name.split('.')[0]
                test_method = test_name.split('.')[1]
                
                print(f"    {Colors.RED}{Colors.BOLD}[{test_counter}] {test_name}{Colors.RESET}")
                print(f"        Suite:    {suite_name}")
                print(f"        Category: {category}")
                print(f"        Method:   {test_method}")
                
                # Print error details if available
                if test_name in self.parser.test_errors:
                    errors = self.parser.test_errors[test_name]
                    if errors:
                        print(f"        {Colors.DIM}Error Details:{Colors.RESET}")
                        for error_line in errors:  # Show all error lines
                            print(f"          {Colors.YELLOW}{error_line}{Colors.RESET}")
                print()
                test_counter += 1
            
            print()  # Blank line between categories
        
        # Print uncategorized tests if any
        if 'Uncategorized' in tests_by_category:
            tests = tests_by_category['Uncategorized']
            print(f"  {Colors.YELLOW}{Colors.BOLD}Uncategorized{Colors.RESET} ({len(tests)} failed):")
            
            for test in tests:
                test_name = test['name']
                suite_name = test_name.split('.')[0]
                test_method = test_name.split('.')[1]
                
                print(f"    {Colors.RED}{Colors.BOLD}[{test_counter}] {test_name}{Colors.RESET}")
                print(f"        Suite:   {suite_name}")
                print(f"        Method:  {test_method}")
                
                # Print error details if available
                if test_name in self.parser.test_errors:
                    errors = self.parser.test_errors[test_name]
                    if errors:
                        print(f"        {Colors.DIM}Error Details:{Colors.RESET}")
                        for error_line in errors:
                            print(f"          {Colors.YELLOW}{error_line}{Colors.RESET}")
                print()
                test_counter += 1
    
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
