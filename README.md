# Turing machine simulator

---

# General info

---

This is implementation of turing machine simulator in C language.

# Description of use

---

Program starts this way:

```csharp
C:\Users\FrantaPepaJednicka> TuringMachine.exe palindromes.tm
```

When you want to close program, you can write  ":quit" in the place of input.

## Pattern for .tm files

---

In .tm files one must declare initial state, final states and transition rules.

- Initial state

    Initial state must be declared at the first line as a first (single) character. When there

    is no new line immidiately after the initial state char, it is recognized as error.

- Second line of the file is reserved for final states (Each final state must be also single character).

    When there is no final state in the machine, line can be empty, but not omitted.

    Individual states are not divided by any character.

    There must be blank line after the final states section

    - only exception is when there are no transition rules.
- Each transition rule is on its own separate line.

     Transition rule is in format:

    ```csharp
    (from-state, read) -> (to-state, write, direction)
    ```

    where each part (from-state, read, to-state, write, direction) is single character and none of the parts contain one of reserved chars ("("    ")"    " "    ","    "<"    ">"    "_" ). There is only one exception to this rule: direction have to be either "<", ">" or "_".