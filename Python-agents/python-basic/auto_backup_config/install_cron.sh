#!/usr/bin/env bash
# Install a cron job to run the backup every Sunday at 00:00 (midnight).

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SCRIPT_PATH="$SCRIPT_DIR/auto_backup_config.py"
LOG_PATH="$HOME/.cache/auto_backup_config.log"

CRON_LINE="0 0 * * 0 /usr/bin/env python3 $SCRIPT_PATH >> $LOG_PATH 2>&1"

TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

# Start with existing crontab (if any)
crontab -l 2>/dev/null > "$TMP" || true

# Add the line only if it's not already present
grep -F "$SCRIPT_PATH" "$TMP" >/dev/null || echo "$CRON_LINE" >> "$TMP"

crontab "$TMP"
echo "Installed cron job: $CRON_LINE"
echo "Log file: $LOG_PATH"
