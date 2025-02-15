# simplq-cli
A command line tool to access the SimplQ API.

```
Usage: simplqclient <command> [options]
Commands:
  get_all_queues
  get_all_entries <queue_id>
  get_pending_entries <queue_id>
  get_processing_entries <queue_id>
  get_completed_entries <queue_id>
  get_error_entries <queue_id>
  get_next_entry <queue_id>
  get_entry <queue_id> <entry_id>
  create_entry <queue_id> <payload>
  update_entry <queue_id> <entry_id> <json_attribute_values>
  set_entry_pending <queue_id> <entry_id> <string_message>
  set_entry_processing <queue_id> <entry_id> <string_message>
  set_entry_complete <queue_id> <entry_id> <string_message>
  set_entry_error <queue_id> <entry_id> <string_message>
  create_queue <json_payload>
  update_queue <queue_id> <json_attribute_values>
  delete_queue <queue_id>
Options:
  --config <file>   Specify a configuration file (default: simplqclient.config)
```
