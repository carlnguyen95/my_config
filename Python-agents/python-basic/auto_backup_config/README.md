# auto_backup_config

This project provides a small script to back up your `~/.config` directory to `~/.snap/.config` and commit changes to a local git repository. It is intended to be run regularly (for example from cron).

Installation

- Ensure Python 3 and `git` are installed. `rsync` is recommended but optional.
- From this repository run the installer to add a cron job that runs every Sunday at 00:00 (midnight):

```bash
chmod +x install_cron.sh
./install_cron.sh
```

This will add a cron entry that runs the script and appends output to `~/.cache/auto_backup_config.log`.

Manual cron line

If you prefer to edit your crontab manually, add this line (replace the path with the absolute path to the script):

```
0 0 * * 0 /usr/bin/env python3 /path/to/auto_backup_config.py >> $HOME/.cache/auto_backup_config.log 2>&1
```

Notes

- The first run will initialize a git repository at `~/.snap/.config` and set a local `user.name`/`user.email` for commits.
- The script commits only local changes; it does not push to any remote. If you want remote backups, add a remote and push from this repository.
- Running from cron will use a minimal environment; the script uses full paths where appropriate and configures a local git user for commits.
