# Trading Engine CLI

## 1. General Syntax
<command> <sub-command> <arguments>

<command> ::= <utility-command> 
            | <data-command> 
            | <indicator-command> 
            | <strategy-command> 
            | <risk-command> 
            | <backtest-command>

<argument> ::= <number>
             | <identifier>
             | <string>

<arguments> ::= <argument>*

## 2. Token Rules

### Identifier

Identifiers consist of letters, digits, `_`, and `-`, and must begin with a letter.

Examples:

EMA
SMA
EURUSD
my_strategy
strategy-name

### Number

Numbers may be integers or floating-point numbers.

Examples:

10
14
0.5
12.75

### String

Strings are enclosed in double quotes.

Examples:

"EURUSD.csv"
"my strategy"

### Whitespace

Whitespace separates tokens and is otherwise ignored.

### End of Input

Every input terminates with an END token.

## 3. Utility Commands
help
exit
summary
logs

<utility-command> ::= "help"
                    | "exit"
                    | "summary"
                    | "logs"

## 4. Data Commands
data load <symbol> <path>
data download <symbol>
data list
data status

<symbol> = identifier
<path>   = string

<data-command> ::=
                 | "data" "load" <identifier> <string>
                 | "data" "download" <identifier>
                 | "data" "list"
                 | "data" "status"

## 5. Indicator Commands
indicator add <indicator_name> <arguments>
indicator list # list created indicators
indicator list available # list available indicators to create
indicator remove <id>
indicator configure <id> <arguments> 


<indicator_name> = identifier
<id> = number

<indicator-command> ::= "indicator" "add" <identifier> <arguments>
                       | "indicator" "list"
                       | "indicator" "list" "available"
                       | "indicator" "remove" <number>
                       | "indicator" "configure" <number> <arguments>


## 6. Strategy Commands
strategy set <strategy-name>
strategy configure <strategy-setting> <arguments>
strategy list 
strategy info 

<strategy-name> = identifier
<strategy-setting> = identifier

<strategy-command> ::= "strategy" "set" <identifier>
                     | "strategy" "configure" <identifier> <arguments>
                     | "strategy" "list"
                     | "strategy" "info"

## 7. Risk Commands
risk set <percent>
risk info

<percent> = number

<risk-command> ::= "risk" "set" <number>
                 | "risk" "info"

## 8. Backtesting Commands
backtest test <strategy-name>
backtest result

<strategy-name> = identifier

<backtest-command> ::= "backtest" "test" <identifier>
                     | "backtest" "result"

## 9. Execution Commands

## 10. Reporting Commands

## 11. Logging Commands

## 12. Errors

## 13. Examples