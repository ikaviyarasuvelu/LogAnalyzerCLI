# Log Analyzer CLI: Design

Based on `requirements.md`. This file says what each piece takes in and gives out. It has no code.

## 1. Data we pass around

| Name | Fields | Used for |
|------|--------|----------|
| **LineRecord** | line number, line text | One line from the file |
| **Entry** | line number, IP, status code, method, path | One successfully parsed line |
| **Summary** | total lines, valid lines, skipped lines, IP counts, status counts, error entries | Everything the report needs |

- **IP counts:** a map of IP to number of requests.
- **Status counts:** a map of status code to number of lines.
- **Error entries:** a list of Entry, in file order.

## 2. Pieces

### Piece 1: Read file
- **Input:** file path
- **Output:** LineRecords, one by one (line number starts at 1)
- **Fails with:**
  - file not found: exit code 2
  - cannot open: exit code 2
  - file is empty: exit code 3
- Line numbers are given here because only the reader knows which line it is on.

### Piece 2: Parse line
- **Input:** one LineRecord
- **Output:** either an **Entry** (valid) or **INVALID** (bad line)
- Takes out: IP, status code, method, path. Timestamp and bytes are ignored.
- INVALID means the line did not match the format in `requirements.md`.

### Piece 3: Is error?
- **Input:** status code only
- **Output:** true or false
- Rule: `status >= 400` is true.
- Does not need IP or path, so it does not take them.

### Piece 4: Collect statistics
- **Input:** the results of Piece 2 and 3, one line at a time
  - valid Entry plus its error flag, or
  - an INVALID notice
- **Work done:**
  - total lines + 1 for every line
  - valid lines + 1 for every Entry
  - skipped lines + 1 for every INVALID
  - IP count + 1 for the Entry's IP
  - status count + 1 for the Entry's status
  - if error flag is true, add the Entry to error entries
- **Output (after all lines):** the **Summary**
- Counts only. No sorting and no printing here.

### Piece 5: Report
- **Input:** Summary and the file name (for the header)
- **Work done:**
  - sort IPs by count, highest first (tie: lower IP text first)
  - sort status codes ascending
  - keep error entries in file order
- **Output:** the report printed to stdout, in the format shown in `requirements.md`
- Sorting is here, not in Piece 4, because sorting only affects how the output looks. This keeps Piece 4 simple to test.

## 3. Main flow (ties the pieces together)

```
read command-line argument
    none given            -> stderr message, exit 1

open file (Piece 1)
    not found / no access -> stderr message, exit 2
    empty                 -> stderr message, exit 3

for each LineRecord from Piece 1:
    result = Parse(LineRecord)            (Piece 2)
    if result is INVALID:
        tell Piece 4: one skipped line
    else:
        flag = IsError(result.status)     (Piece 3)
        tell Piece 4: entry + flag

after all lines:
    if valid lines == 0   -> stderr message, exit 4
    Report(Summary, file name)            (Piece 5)
    exit 0
```

## 4. Chain check (output of one piece matches input of the next)

| From | To | Passed along | Matches? |
|------|----|--------------|----------|
| Piece 1 | Piece 2 | LineRecord (number + text) | Yes |
| Piece 2 | Piece 3 | status code from the Entry | Yes |
| Piece 2 + 3 | Piece 4 | Entry + error flag, or INVALID | Yes |
| Piece 4 | Piece 5 | Summary | Yes |
| Command line | Piece 5 | file name for the header | Yes |

## 5. Design decisions
1. **Bad lines are skipped, not fatal.** One broken line should not stop the whole analysis (requirements, case e).
2. **Counting and sorting are separate.** Piece 4 counts, Piece 5 sorts.
3. **Piece 3 takes only the status code.** Smallest possible input.
4. **Everything is read line by line.** The whole file is never loaded into memory at once, so large logs work.
5. **Errors go to stderr, the report goes to stdout.** The report can be redirected to a file without error messages mixed in.

## 6. How each piece will be tested

| Piece | Test idea |
|-------|-----------|
| 1 | Normal file, missing file, empty file. Line numbers start at 1 |
| 2 | Good line, line with a missing field, empty text, random text |
| 3 | 200, 399, 400, 404, 500 |
| 4 | The 6-line sample in `requirements.md` gives 5 valid, 1 skipped, 3 errors |
| 5 | The same sample gives exactly the expected output |
