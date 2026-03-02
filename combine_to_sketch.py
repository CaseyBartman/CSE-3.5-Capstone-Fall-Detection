import os
import re
import sys
from pathlib import Path

# Directories to search for source files
HEADER_DIRS = [
    'include/constants',
    'include/interfaces',
    'include/models',
    'test/generatorss',
    'test/helpers',
]
SRC_DIRS = [
    'src/drivers/real',
    'src/drivers/sim',
    'src/logic',
]
LOGIC_DIR = 'src/logic'
MAIN_FILE = 'src/main.cpp'
OUTPUT_FILE = 'sketch.ino'

def get_files(dirs, exts):
    """Recursively get all files matching extensions in given directories."""
    files = []
    for d in dirs:
        if not os.path.isdir(d):
            continue
        for f in sorted(os.listdir(d)):
            if any(f.endswith(ext) for ext in exts):
                files.append(os.path.join(d, f))
    return files

def strip_local_includes(code):
    """Remove #include "..." lines (local includes)."""
    return re.sub(r'^\s*#include\s+"[^"]+"\s*$', '', code, flags=re.MULTILINE)

def read_and_process(file_path):
    """Read a file and strip local includes."""
    with open(file_path, 'r', encoding='utf-8') as f:
        code = f.read()
        code = strip_local_includes(code)
        return code

def main():
    # Parse command-line arguments
    is_simulation = True  # Default to simulation
    
    if len(sys.argv) > 1:
        if sys.argv[1] == '--production':
            is_simulation = False
        elif sys.argv[1] == '--simulation':
            is_simulation = True
        else:
            print("Usage: python combine_to_sketch.py [--simulation|--production]")
            print("  --simulation: Generate for Wokwi simulation (default)")
            print("  --production: Generate for real hardware")
            sys.exit(1)
    
    output = []
    
    # Add IS_SIMULATION preprocessor define at the top
    output.append('// Auto-generated sketch.ino - Do not edit directly\n')
    if is_simulation:
        output.append('#define IS_SIMULATION 1\n')
        output.append('// Running in SIMULATION mode (Wokwi)\n\n')
    else:
        output.append('#define IS_SIMULATION 0\n')
        output.append('// Running in PRODUCTION mode (Real Hardware)\n\n')

    # 1. Add all header files from include/ (interfaces, models, constants)
    header_files = get_files(HEADER_DIRS, ['.h'])
    for file in header_files:
        code = read_and_process(file)
        output.append(f'// ===== {file} =====\n')
        output.append(code + '\n')

    # 2. Add FallDetector.h from src/logic (class declaration)
    falldetector_h = os.path.join(LOGIC_DIR, 'FallDetector.h')
    if os.path.exists(falldetector_h):
        code = read_and_process(falldetector_h)
        output.append(f'// ===== {falldetector_h} =====\n')
        output.append(code + '\n')

    # 3. Add all driver .cpp files (implementations)
    driver_files = get_files(['src/drivers/real', 'src/drivers/sim'], ['.cpp'])
    for file in driver_files:
        code = read_and_process(file)
        output.append(f'// ===== {file} =====\n')
        output.append(code + '\n')

    # 4. Add FallDetector.cpp from src/logic (class implementation)
    falldetector_cpp = os.path.join(LOGIC_DIR, 'FallDetector.cpp')
    if os.path.exists(falldetector_cpp):
        code = read_and_process(falldetector_cpp)
        output.append(f'// ===== {falldetector_cpp} =====\n')
        output.append(code + '\n')

    # 5. Add main.cpp last
    if os.path.exists(MAIN_FILE):
        code = read_and_process(MAIN_FILE)
        output.append(f'// ===== {MAIN_FILE} =====\n')
        output.append(code + '\n')

    # Write to sketch.ino
    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write('\n'.join(output))
    
    mode = "SIMULATION" if is_simulation else "PRODUCTION"
    print(f'✓ Combined sketch written to {OUTPUT_FILE} ({mode} mode)')

if __name__ == '__main__':
    main()