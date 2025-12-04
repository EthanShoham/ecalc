$ErrorActionPreference = "Stop"

$commonFlags = @("-std=c11", "-Wall", "-Wextra", "-Werror")
$sharedSources = @("lexer.c", "char_reader.c", "token_list.c", "list.c")

Write-Host "Building main.exe..."
& gcc @commonFlags @sharedSources "main.c" -o "main.exe"

Write-Host "Building lexer_test.exe..."
& gcc @commonFlags @sharedSources "lexer_test.c" -o "lexer_test.exe"

Write-Host "Running lexer_test.exe..."
& "./lexer_test.exe"