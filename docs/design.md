 Log Analyzer CLI: Design

Based on `requirements.md`. This file says what each piece takes in and gives out. It has no code.

 1. Data we pass around

| Name | Fields | Used for |
|------|--------|----------|
| LineRecord | line number, line text | One line from the file |
| Entry | line number, IP, status code, method, path | One successfully parsed line |
| Summary | total lines, valid lines, skipped lines, IP counts, status counts, error entries | Everything the report needs |

- IP counts: a map of IP text to number of requests. The IP text is used exactly as written (no normalizing).
- Status counts: a map of status code to number of lines.
- Error entries: a list of Entry, in file order.
- Entry fields have safe default values, so a half-filled Entry never holds garbage.

 2. Pieces

 Piece 1: Read file
- Input: file path
- Output: LineRecords, one by one (line number starts at 1)
- Reports a failure of one of these kinds (it does not choose exit codes; the main flow does):
  - file not found
  - path is a directory
  - cannot open
  - file is empty
  - read error while reading
- Line numbers are given here because only the reader knows which line it is on.
- Blank lines are still passed on as LineRecords. Piece 2 decides they are bad lines.
- A trailing `\r` is left in the line text. Piece 2 allows it.

 Piece 2: Parse line
- Input: one LineRecord
- Output: either an Entry (valid) or INVALID (bad line)
- Takes out: IP, status code, method, path. The line number is copied from the LineRecord to the Entry.
- Timestamp and bytes are not used, but they must still follow their rules in `requirements.md` section 2.2.
- INVALID means the line did not follow every rule in `requirements.md` section 2.
- Fields are checked left to right. The first failed check gives INVALID, and the Entry must not be used.

Small helpers inside Piece 2 (not separate pieces in the main flow; each takes text and gives true or false, so each can be tested alone):

| Helper | Input | Output | Rule |
|--------|-------|--------|------|
| Is valid IP | IP text | true or false | Text has `:` means check as IPv6, otherwise check as IPv4 |
| Is valid IPv4 | IP text | true or false | `requirements.md` 2.3 |
| Is valid IPv6 | IP text | true or false | `requirements.md` 2.4 |
| Is valid status | status text | true or false, plus the number if true | Exactly 3 digits, 100 to 999 |

 Piece 3: Is error?
- Input: status code only
- Output: true or false
- Rule: `status >= 400` is true.
- Does not need IP or path, so it does not take them.

 Piece 4: Collect statistics
- Input: the results of Piece 2 and 3, one line at a time
  - valid Entry plus its error flag, or
  - an INVALID notice
- Work done:
  - total lines + 1 for every line
  - valid lines + 1 for every Entry
  - skipped lines + 1 for every INVALID
  - IP count + 1 for the Entry's IP
  - status count + 1 for the Entry's status
  - if error flag is true, add the Entry to error entries
- Output (after all lines): the Summary
- Counts only. No sorting and no printing here.

 Piece 5: Report
- Input: Summary and the file path
- Work done:
  - take the file name from the path (without the folder; both `/` and `\` separate folders)
  - sort IPs by count, highest first (tie: lower IP text first)
  - sort status codes ascending
  - keep error entries in file order
- Output: the report printed to stdout, in the format shown in `requirements.md`
- Sorting is here, not in Piece 4, because sorting only affects how the output looks. This keeps Piece 4 simple to test.

 3. Main flow (ties the pieces together)


read command-line argument
    none given            -> stderr message, exit 1

open file (Piece 1)
    not found             -> stderr message, exit 2
    is a directory        -> stderr message, exit 2
    cannot open           -> stderr message, exit 2
    empty                 -> stderr message, exit 3

for each LineRecord from Piece 1:
    result = Parse(LineRecord)            (Piece 2)
    if result is INVALID:
        tell Piece 4: one skipped line
    else:
        flag = IsError(result.status)     (Piece 3)
        tell Piece 4: entry + flag
    read error            -> stderr message, no report, exit 5

after all lines:
    if valid lines == 0   -> stderr message, exit 4
    Report(Summary, file path)            (Piece 5)
    exit 0


 4. Chain check (output of one piece matches input of the next)

| From | To | Passed along | Matches? |
|------|----|--------------|----------|
| Piece 1 | Piece 2 | LineRecord (number + text) | Yes |
| Piece 2 | Piece 3 | status code from the Entry | Yes |
| Piece 2 + 3 | Piece 4 | Entry + error flag, or INVALID | Yes |
| Piece 4 | Piece 5 | Summary | Yes |
| Command line | Piece 5 | file path (Piece 5 takes the name for the header) | Yes |

 5. Design decisions
1. Bad lines are skipped, not fatal. One broken line should not stop the whole analysis (requirements, case f).
2. Counting and sorting are separate. Piece 4 counts, Piece 5 sorts.
3. Piece 3 takes only the status code. Smallest possible input.
4. Everything is read line by line. The whole file is never loaded into memory at once, so large logs work.
5. Errors go to stderr, the report goes to stdout. The report can be redirected to a file without error messages mixed in.
6. Pieces report failures, the main flow decides exit codes. Piece 1 says what went wrong. Only the main flow knows the exit codes and messages.
7. Piece 2 uses small checking helpers. IPv4, IPv6, and status checks are separate, so each can be tested alone and the parser stays readable.
8. A failed parse never leaves usable data. INVALID means the Entry must not be used, and Entry defaults keep it safe if it is.

 6. How each piece will be tested

| Piece | Test idea |
|-------|-----------|
| 1 | Normal file, missing file, directory, empty file, file with a trailing blank line. Line numbers start at 1 |
| 2 | All examples in `requirements.md` section 2.6: good lines, missing field, empty text, spaces-only, random text, `-` bytes, `\r` at the end, extra text, leading space |
| IP helpers | Every IP row in `requirements.md` section 2.6, plus extra IPv6 cases |
| Status helper | Every status row in `requirements.md` section 2.6 |
| 3 | 100, 200, 399, 400, 404, 500, 999 |
| 4 | The 6-line sample in `requirements.md` gives 5 valid, 1 skipped, 3 errors |
| 5 | The same sample gives exactly the expected output. Also a path with folders gives only the file name in the header |
