# CLAUDE.md - AI Assistant Guide for simplq-cli

## Project Overview

**simplq-cli** is a command-line interface tool written in C that provides access to the SimplQ queue management API. It allows users to manage queues and entries from the terminal.

- **Language**: C
- **License**: MIT
- **External Dependency**: libcurl (for HTTP requests)

## Codebase Structure

```
simplq-cli/
├── src/
│   └── main.c              # Complete CLI implementation (single file)
├── MakeFile                # Build configuration
├── README.md               # User documentation
├── simplqclient.config.example  # Configuration template
├── LICENSE                 # MIT license
└── .gitignore              # Git ignore rules
```

This is a minimal, single-file C project. All source code is in `src/main.c`.

## Build Commands

```bash
# Build the project
make

# Clean build artifacts
rm -f simplq-cli
# or: make clean
```

**Compiler**: GCC with flags `-Wall -Wextra -O2`

**Output**: Executable `simplq-cli` in project root

**Linking**: Requires `-lcurl`

## Configuration

The CLI reads configuration from a key=value file (default: `simplqclient.config`):

```
API_KEY=<your_api_key>
API_VERSION=v1
API_SERVER=api.simplq.dev
```

Override with `--config <file>` flag before the command.

## Code Architecture

### Key Structures (src/main.c)

- **SimplQClient** (line 9-14): Holds API configuration (api_key, api_version, api_server, user_agent)
- **ResponseBuffer** (line 16-19): Dynamic buffer for HTTP responses

### Core Functions

| Function | Line | Purpose |
|----------|------|---------|
| `write_callback()` | 21 | libcurl callback for response accumulation |
| `build_url()` | 39 | Constructs API endpoint URLs |
| `send_request()` | 43 | Performs HTTP requests with headers |
| `load_config()` | 89 | Parses configuration file |
| `print_help()` | 115 | Returns JSON-formatted help text |
| `main()` | 160 | Command router and entry point |

### Command Handling Pattern

Commands are handled in a long if/else chain starting at line 186. Each block:
1. Checks command name (supports aliases like `qs` for `get_all_queues`)
2. Validates required arguments
3. Builds endpoint string
4. Calls `send_request()` with appropriate HTTP method

## Code Conventions

### Style
- Opening braces on same line for command handlers, new line for closing
- Fixed buffer size constant: `BUFFER_SIZE = 2048`
- Error messages output as JSON: `{ "error": "message" }`
- Memory manually managed with malloc/free/realloc

### HTTP Methods Used
- **GET**: Read operations
- **POST**: Create operations
- **PATCH**: Update operations
- **DELETE**: Remove operations

### Command Aliases
| Full Command | Alias |
|--------------|-------|
| get_all_queues | qs |
| get_queue | q |
| create_queue | cq |
| update_queue | uq |
| delete_queue | dq |
| get_all_entries | es |
| get_entry | e |
| create_entry | ce |
| delete_entry | de |

## Adding New Commands

To add a new command:

1. Add an `else if` block in `main()` following the existing pattern:
```c
else if (strcmp(command, "new_command") == 0 || strcmp(command, "nc") == 0)
    {
    if (argc < N) {
        printf("%s", "{ \"error\": \"Description of required args\" }");
        return 1;
    }
    char endpoint[BUFFER_SIZE];
    snprintf(endpoint, BUFFER_SIZE, "path/%s", argv[2]);
    response = send_request(&client, "METHOD", endpoint, payload_or_NULL);
    }
```

2. Update `print_help()` to include the new command
3. Update `README.md` with usage documentation

## Testing

**No automated testing framework**. Test manually by running commands:

```bash
# Build first
make

# Example commands (requires valid config)
./simplq-cli get_all_queues
./simplq-cli create_queue '{"name":"test"}'
./simplq-cli get_queue <queue_id>
```

## Important Notes for AI Assistants

1. **Single file codebase**: All logic is in `src/main.c` - no header files or modules
2. **Buffer limits**: Fixed 2048-byte buffers; be aware of potential overflow for very long inputs
3. **No JSON parsing**: The CLI passes JSON payloads directly to the API without validation
4. **Memory management**: Ensure all malloc'd memory is freed (see cleanup at end of main())
5. **Error handling**: Use JSON-formatted error messages for consistency
6. **Config files are gitignored**: Never commit actual config files with API keys
7. **Response output**: API responses are printed to stdout with a newline

## Known Issues

- Line 265: Error message says "completed" instead of "error" for `get_error_entries`
- `print_help()` return value is not used in some code paths (lines 162-163, 173-174)
- Missing commas in `print_help()` JSON output (lines 133, 135) make it invalid JSON
