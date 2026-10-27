 Log Analyzer CLI: Requirements

 1. Purpose
    A command-line program that reads one web server log file and prints a summary: requests per IP, status code counts, and the list of error lines.

 2. Input

   One log file path, given as a command-line argument.
   Example: `analyzer C:/programs/log.txt`
   Only one file per run.

   Log line format
   Each line must look like this:

   <IP> - - [<timestamp>] "<METHOD> <path> HTTP/<version>" <status> <bytes>
   
   Example:
    
   192.168.1.10 - - [25/Oct/2026:10:15:01] "GET /index.html HTTP/1.1" 200 1024

   IP is the first field.
   Status code is the number after the closing quote.

 3. Definitions

   Error line: a line whose status code is 400 or above (4xx and 5xx).
   Valid line: a line that matches the format above.
   Bad line: a line that does not match the format (counted and skipped).

 4. What the program counts

   1. Requests per unique IP.
   2. Number of lines for each status code.
   3. Error lines: line number, status code, IP, request, plus the total error count.
   4. Total lines, valid lines, and skipped (bad) lines.

 5. Output (printed to screen)

   Sample input (`log.txt`, 6 lines; line 6 is a bad line)

   192.168.1.10 - - [25/Oct/2026:10:15:01] "GET /index.html HTTP/1.1" 200 1024
   192.168.1.11 - - [25/Oct/2026:10:15:02] "GET /login HTTP/1.1" 404 512
   192.168.1.10 - - [25/Oct/2026:10:15:03] "POST /api/data HTTP/1.1" 500 128
   192.168.1.12 - - [25/Oct/2026:10:15:04] "GET /index.html HTTP/1.1" 200 1024
   192.168.1.10 - - [25/Oct/2026:10:15:05] "GET /missing HTTP/1.1" 404 256
   this line is broken
   

   Expected output:

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
  

   Ordering rules:
     - IPs: highest count first. If counts are equal, lower IP (sorted as text) first.
     - Status codes: ascending by code.
     - Error lines: in file order.

 6. Edge cases and errors

    |   | Situation                                        | Behaviour                                                            | Exit code |
    |---|--------------------------------------------------|----------------------------------------------------------------------|-----------|
    | a | No file path given                               | stderr: `Error: please provide the log file path. Usage: analyzer <logfile>` | 1 |
    | b | File path given, file does not exist             | stderr: `Error: file not found: <path>`                                      | 2 |
    | c | File exists but cannot be opened (no permission) | stderr: `Error: cannot open file: <path>`                                    | 2 |
    | d | File is empty                                    | stderr: `Error: file is empty`                                               | 3 |
    | e | Some lines do not match the format               | Skip them, continue, and show the skipped count in the summary               | 0 |
    | f | No line matches the format                       | stderr: Error: no valid log lines found.                                     | 4 |

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
