 Log Analyzer CLI: Requirements

 1. Purpose

   A command-line program that reads one web server log file and prints a summary: requests per IP, status code counts, and the list of error lines.

 2. Input

   One log file path, given as a command-line argument.
   Example: `analyzer C:/programs/log.txt`
  Only one file per run.

  2.1 Log line format

    Each line must look like this:

      <IP> - - [<timestamp>] "<METHOD> <path> HTTP/<version>" <status> <bytes>

     Example:

      192.168.1.10 - - [25/Oct/2026:10:15:01] "GET /index.html HTTP/1.1" 200 1024 

     IP is the first field. Status code is the number after the closing quote.

 2.2 Field rules

   A line is valid only if every field follows its rule. Fields are separated by exactly one space, as shown in the format.

   | Field | Rule |
   |-------|------|
   | IP | IPv4 or IPv6, as defined in 2.3 and 2.4 |
   | ident and user | Exactly the text `- -` (dash, space, dash) |
   | timestamp | Inside `[` and `]`, at least one character. Not checked further. Ignored by the program |
   | method | One or more letters A to Z (any method, for example GET, POST, DELETE) |
   | path | One or more characters. No spaces and no `"` |
   | version | `HTTP/` followed by digits, optionally `.` and more digits (1.0, 1.1, 2, 2.0) |
   | status | Exactly three digits, value 100 to 999. No sign, no spaces, no leading zero |
   | bytes | Digits only, or a single `-`. Ignored by the program |

Whole-line rules:
- A line must not start with a space.
- Spaces, tabs, and a carriage return (`\r`, from Windows line endings) after the last field are allowed and ignored.
- Any other text after the bytes field makes the line a bad line.
- A blank line, or a line with only spaces, is a bad line. It is counted in total lines and in skipped lines.

 2.3 IPv4 rules

- Exactly 4 parts, separated by `.`
- Each part has 1 to 3 digits and nothing else
- Each part has a value from 0 to 255
- No leading zero, except the part that is exactly `0`

 2.4 IPv6 rules

- Groups separated by `:`. Each group has 1 to 4 hex digits (`0-9`, `a-f`, `A-F`)
- Full form: exactly 8 groups
- Compressed form: `::` appears at most once and stands for one or more groups of zeros. With `::`, at most 7 groups are written. `::` alone is valid
- A single `:` at the start or end of the text (not part of `::`) is invalid
- Not supported (the line is a bad line): IPv4 written inside IPv6 (`::ffff:1.2.3.4`), zone ids (`fe80::1%eth0`), square brackets, port numbers

 2.5 IP identity

The IP is counted exactly as written. `ABCD::1` and `abcd::1` are counted as two different IPs. No normalizing or shortening.

 2.6 Validation examples

These are also the test cases for Piece 2.

| IP text | Result | Why |
|---------|--------|-----|
| `192.168.1.10` | valid | normal IPv4 |
| `0.0.0.0` | valid | zero parts are fine |
| `256.1.1.1` | invalid | part above 255 |
| `1.2.3` | invalid | only 3 parts |
| `1.2.3.4.5` | invalid | 5 parts |
| `1..3.4` | invalid | empty part |
| `01.2.3.4` | invalid | leading zero |
| `a.b.c.d` | invalid | not digits |
| `2001:db8:0:0:0:0:0:1` | valid | full IPv6 |
| `2001:db8::1` | valid | compressed |
| `::1` | valid | compressed |
| `::` | valid | all zeros |
| `ABCD::ef01` | valid | hex is case-insensitive |
| `1:2:3:4:5:6:7:8:9` | invalid | 9 groups |
| `1::2::3` | invalid | `::` twice |
| `12345::1` | invalid | group longer than 4 digits |
| `g::1` | invalid | `g` is not hex |
| `:1:2:3:4:5:6:7` | invalid | single colon at start |
| `1:2:3:4:5:6:7:` | invalid | single colon at end |
| `::ffff:1.2.3.4` | invalid | out of scope |

| Status text | Result |
|-------------|--------|
| `200`, `404`, `999`, `100` | valid |
| `99`, `1000` | invalid (not 3 digits) |
| `020` | invalid (leading zero) |
| `abc`, `200abc`, `+200`, `-200`, `2 00` | invalid |

| Whole line | Result |
|------------|--------|
| the sample lines 1 to 5 | valid |
| `this line is broken` | invalid |
| empty line, spaces-only line | invalid |
| valid line with `-` as bytes | valid |
| valid line with `\r` at the end | valid |
| valid line with extra text after bytes | invalid |
| valid line with a space before the IP | invalid |

 3. Definitions

- **Error line:** a line whose status code is 400 or above (4xx and 5xx).
- **Valid line:** a line that follows every rule in section 2.
- **Bad line:** a line that does not (counted and skipped).

 4. What the program counts

1. Requests per unique IP.
2. Number of lines for each status code.
3. Error lines: line number, status code, IP, request, plus the total error count.
4. Total lines (every line, including blank lines), valid lines, and skipped (bad) lines.

 5. Output (printed to screen)

Sample input (`log.txt`, 6 lines; line 6 is a bad line):

```
192.168.1.10 - - [25/Oct/2026:10:15:01] "GET /index.html HTTP/1.1" 200 1024
192.168.1.11 - - [25/Oct/2026:10:15:02] "GET /login HTTP/1.1" 404 512
192.168.1.10 - - [25/Oct/2026:10:15:03] "POST /api/data HTTP/1.1" 500 128
192.168.1.12 - - [25/Oct/2026:10:15:04] "GET /index.html HTTP/1.1" 200 1024
192.168.1.10 - - [25/Oct/2026:10:15:05] "GET /missing HTTP/1.1" 404 256
this line is broken
```

Expected output:

```
=== Log Analysis: log.txt ===
Total lines: 6 (valid: 5, skipped: 1)

Requests per IP (highest first):
192.168.1.10    3
192.168.1.11    1
192.168.1.12    1

Status codes:
200    2
404    2
500    1

Error lines (status >= 400): 3 total
line 2: 404  192.168.1.11  GET /login
line 3: 500  192.168.1.10  POST /api/data
line 5: 404  192.168.1.10  GET /missing
```

Ordering rules:
- IPs: highest count first. If counts are equal, lower IP (sorted as text) first. IPv4 and IPv6 are sorted together as text.
- Status codes: ascending by code.
- Error lines: in file order.

Header rule: the header shows only the file name, without the folder. For `C:/programs/log.txt` or `C:\programs\log.txt` the header shows `log.txt`.

 6. Edge cases and errors

| | Situation | Behaviour | Exit code |
|---|-----------|-----------|-----------|
| a | No file path given | stderr: `Error: please provide the log file path. Usage: analyzer <logfile>` | 1 |
| b | File path given, file does not exist | stderr: `Error: file not found: <path>` | 2 |
| c | Path is a directory, not a file | stderr: `Error: path is a directory: <path>` | 2 |
| d | File exists but cannot be opened (no permission) | stderr: `Error: cannot open file: <path>` | 2 |
| e | File is empty (zero bytes, no lines) | stderr: `Error: file is empty` | 3 |
| f | Some lines do not match the format | Skip them, continue, and show the skipped count in the summary | 0 |
| g | No line matches the format (includes a file with only blank lines) | stderr: `Error: no valid log lines found.` | 4 |
| h | Read error while reading the file | stderr: `Error: failed while reading file`. No report is printed | 5 |

Rules:
- All error messages go to stderr. The summary goes to stdout.
- Exit code 0 means success.

 7. Out of scope

- Multiple files in one run.
- A file editor to fix the format.
- Suggestions on how to fix the errors.
- Dashboard or charts.
- Converting other log formats into this format.
- Reading logs over the network.
- Filtering by date or time range.
- IPv4 written inside IPv6, IPv6 zone ids, port numbers.
- Normalizing IPs (case, leading zeros, shortened forms).

 8. Rule decisions

| Rule | Status |
|------|--------|
| Status code is an integer from 100 to 999 | Decided |
| IP must be strictly checked | Decided |
| IPv4 and IPv6 both supported | Decided |
| Status is exactly 3 digits with no leading zero | Proposed, confirm |
| IPv4: no leading zeros | Proposed, confirm |
| IPv6: `::` compression allowed once, embedded IPv4 and zone ids out of scope | Proposed, confirm |
| IPs counted as written (no normalizing) | Proposed, confirm |
| Bytes may be `-` | Proposed, confirm |
| Method: any letters A to Z | Proposed, confirm |
| Path: no spaces, no `"`, at least one character | Proposed, confirm |
| Version: `HTTP/` plus digits, optional `.digits` | Proposed, confirm |
| ident and user must be exactly `- -` | Proposed, confirm |
| Blank line is a bad line (counted and skipped) | Proposed, confirm |
| Extra text after bytes makes the line bad;
 trailing spaces, tabs and `\r` are fine | Proposed, confirm | | Header shows file name only,
     without folder | Proposed, confirm | | File with only blank lines gives exit 4,
     not 3 | Proposed, confirm | | Read error mid - file prints no report | Proposed, confirm |
