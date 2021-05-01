import csv
from os import get_inheritable

# Note: to be ran from the directory of the python program
generation_line = "// GENERATED VALUES"
settings_file_name = "./faults.csv"
source_file_name = "../Core/Src/fault_library.c"
header_file_name = "../Core/Inc/fault_library.h"
fault_max = 32

rise_phrase = "const uint16_t rise_threshold[FAULT_MAX] = {"
fall_phrase = "const uint16_t fall_threshold[FAULT_MAX] = {"
signal_phrase = "const uint8_t signal_period[FAULT_MAX] = {"
historic_phrase = "#define HISTORIC_INIT "
enable_phrase = "#define ENABLE_INIT "
criticality_phrase = "#define CRITICALITY_INIT "
set_handle_phrase = "const void (*set_handler[FAULT_MAX])() = {"
cont_handle_phrase = "const void (*cont_handler[FAULT_MAX])() = {"
off_handle_phrase = "const void (*off_handler[FAULT_MAX])() = {"

enum_phrase = "typedef enum { "
enum_phrase_end = " } fault_name_t;"

field_names = ['number', 'name', 'enable', 'rise', 'fall', 'period', 'criticality', 'historic', 'set handle', 'cont handle', 'off handle']

# Import csv file
faults = []
settings_fid = open(settings_file_name)
csv_reader = csv.DictReader(settings_fid, delimiter=',', fieldnames=field_names)
for row in csv_reader:
    faults.append(row)
faults.pop(0)

settings_fid.close()

# Deal with rise and fall thresholds
for fault in faults:
    rise_phrase += fault['rise'] + ', '
    fall_phrase += fault['fall'] + ', '
    signal_phrase += fault['period'] + ', '
rise_phrase = rise_phrase[:-2] + '};'
fall_phrase = fall_phrase[:-2] + '};'
signal_phrase = signal_phrase[:-2] + '};'

# Deal with handlers
for fault in faults:
    set_handle_phrase += fault['set handle'] + ', '
    cont_handle_phrase += fault['cont handle'] + ', '
    off_handle_phrase += fault['off handle'] + ', '
set_handle_phrase = set_handle_phrase[:-2] + '};'
cont_handle_phrase = cont_handle_phrase[:-2] + '};'
off_handle_phrase = off_handle_phrase[:-2] + '};'

# Deal with enum for fault names
for fault in faults:
    if(len(fault['name']) > 0):
        enum_phrase += fault['name'] + '_FAULT_NUM' + ', '
enum_phrase = enum_phrase[:-2] + enum_phrase_end

# Deal with historic
blank_entries = fault_max - len(faults)
binary_phrase = '0' * blank_entries
for i in range(len(faults) - 1, -1, -1):
    if('OVERRIDE' in faults[i]['historic']):
        binary_phrase += '1'
    else:
        binary_phrase += '0'
historic_phrase += hex(int(binary_phrase, 2))

# Deal with enable
binary_phrase = '0' * blank_entries
for i in range(len(faults) - 1, -1, -1):
    if('ENABLE' in faults[i]['enable']):
        binary_phrase += '1'
    else:
        binary_phrase += '0'
enable_phrase += hex(int(binary_phrase, 2))

# Deal with criticality
binary_phrase = '0' * blank_entries
for i in range(len(faults) -1, -1, -1):
    if('CRITICAL' in faults[i]['criticality']):
        binary_phrase += '10'
    elif('ERROR' in faults[i]['criticality']):
        binary_phrase += '01'
    else:
        binary_phrase += '00'
criticality_phrase += hex(int(binary_phrase, 2))

# Import source file
start_line = 0
source_fid = open(source_file_name, "r")
source_content = source_fid.readlines()
source_fid.close()

# Search for line to edit
line_found = False
for line in source_content:
    start_line += 1
    if generation_line in line:
        line_found = True
        break

# Replace lines with auto-generated ones
if(line_found):

    print("Generating on line " + str(start_line))

    source_content[start_line] = rise_phrase + '\n'
    source_content[start_line + 1] = fall_phrase + '\n'
    source_content[start_line + 2] = signal_phrase + '\n'
    source_content[start_line + 3] = set_handle_phrase + '\n'
    source_content[start_line + 4] = cont_handle_phrase + '\n'
    source_content[start_line + 5] = off_handle_phrase + '\n'

    source_fid = open(source_file_name, "w")
    source_fid.writelines(source_content)
else:
    print("Generation line not found.")

source_fid.close()

# Import header file
start_line = 0
header_fid = open(header_file_name, "r")
header_content = header_fid.readlines()
header_fid.close()

# Search for line to edit
line_found = False
for line in header_content:
    start_line += 1
    if generation_line in line:
        line_found = True
        break

# Replace lines with auto-generated ones
if(line_found):

    print("Generating on line " + str(start_line))

    header_content[start_line] = enum_phrase + '\n'
    header_content[start_line + 1] = historic_phrase + '\n'
    header_content[start_line + 2] = enable_phrase + '\n'
    header_content[start_line + 3] = criticality_phrase + '\n'

    header_fid = open(header_file_name, "w")
    header_fid.writelines(header_content)
else:
    print("Generation line not found.")

header_fid.close()