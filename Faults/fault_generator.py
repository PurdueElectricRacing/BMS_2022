import csv

# Note: to be ran from the directory of the python program
generation_line = "// GENERATED VALUES"
settings_file_name = "./faults.csv"
source_file_name = "../Core/Src/fault_library.c"
header_file_name = "../Core/Inc/fault_library.h"
fault_max = 32

rise_phrase = "const uint16_t rise_init[FAULT_MAX] = {"
fall_phrase = "const uint16_t fall_init[FAULT_MAX] = {"
historic_phrase = "#define HISTORIC_INIT "
enable_phrase = "#define ENABLE_INIT "
criticality_phrase = "#define CRITICALITY_INIT "
set_handle_phrase = "const void (*set_handler_init[FAULT_MAX])() = {"
cont_handle_phrase = "const void (*cont_handler_init[FAULT_MAX])() = {"
off_handle_phrase = "const void (*off_handler_init[FAULT_MAX])() = {"

enum_phrase = "typedef enum { "
enum_phrase_end = " } fault_name_t;"

field_names = ['number', 'name', 'enable', 'rise', 'fall', 'criticality', 'historic', 'set handle', 'cont handle', 'off handle']

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
rise_phrase = rise_phrase[:-2] + '};'
fall_phrase = fall_phrase[:-2] + '};'

# Deal with historic
blank_entries = fault_max - len(faults)
historic_phrase += '0b' + '0' * blank_entries
for i in range(len(faults) - 1, -1, -1):
    if('OVERRIDE' in faults[i]['historic']):
        historic_phrase += '1'
    else:
        historic_phrase += '0'

# Deal with enable
enable_phrase += '0b' + '0' * blank_entries
for i in range(len(faults) - 1, -1, -1):
    if('ENABLE' in faults[i]['enable']):
        enable_phrase += '1'
    else:
        enable_phrase += '0'

# Deal with criticality
criticality_phrase += '0b' + '0' * blank_entries * 2
for i in range(len(faults) -1, -1, -1):
    if('CRITICAL' in faults[i]['criticality']):
        criticality_phrase += '10'
    elif('ERROR' in faults[i]['criticality']):
        criticality_phrase += '01'
    else:
        criticality_phrase += '00'

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
    source_content[start_line + 2] = historic_phrase + '\n'
    source_content[start_line + 3] = enable_phrase + '\n'
    source_content[start_line + 4] = criticality_phrase + '\n'
    source_content[start_line + 5] = set_handle_phrase + '\n'
    source_content[start_line + 6] = cont_handle_phrase + '\n'
    source_content[start_line + 7] = off_handle_phrase + '\n'

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

    header_fid = open(header_file_name, "w")
    header_fid.writelines(header_content)
else:
    print("Generation line not found.")

header_fid.close()