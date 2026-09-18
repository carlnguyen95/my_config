#!/usr/bin/env python3
"""Backup ~/.config to ~/.snap/.config and commit changes to a local git repo.

This script is intended to be run from cron. It prefers `rsync` when
available for efficient syncing; falls back to a copy when rsync is missing.
After syncing it initializes a git repository (if needed), stages all
changes and commits with a timestamped message.
"""
from __future__ import annotations

import datetime
import logging
import os
import shutil
import subprocess
import sys

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s: %(message)s")


def run(cmd, cwd=None):
	return subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)


def main() -> int:
	home = os.path.expanduser("~")
	src = os.path.join(home, ".config")
	dest = os.path.join(home, ".snap", ".config")

	if not os.path.exists(src):
		logging.error("Source folder does not exist: %s", src)
		return 1

	os.makedirs(os.path.dirname(dest), exist_ok=True)

	if shutil.which("rsync"):
		logging.info("Syncing with rsync: %s -> %s", src, dest)
		res = run(["rsync", "-a", "--delete", f"{src}/", f"{dest}/"])
		if res.returncode != 0:
			logging.error("rsync failed: %s", res.stderr.strip())
			return 2
	else:
		logging.info("rsync not available; copying files (may be slower)")
		if os.path.exists(dest):
			# Remove destination and copy fresh to ensure deletes are reflected
			shutil.rmtree(dest)
		shutil.copytree(src, dest, dirs_exist_ok=True)

	git = shutil.which("git")
	if not git:
		logging.error("git not found in PATH; cannot commit")
		return 3

	# Initialize repo if needed and set local user config so cron can commit
	if not os.path.exists(os.path.join(dest, ".git")):
		logging.info("Initializing git repository in %s", dest)
		run([git, "init"], cwd=dest)
		run([git, "config", "user.name", "auto-backup"], cwd=dest)
		run([git, "config", "user.email", "auto-backup@localhost"], cwd=dest)

	run([git, "add", "-A"], cwd=dest)
	now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
	commit = run([git, "commit", "-m", f"Backup: {now}"], cwd=dest)
	if commit.returncode == 0:
		logging.info("Committed backup: %s", now)
	else:
		out = (commit.stdout or "") + (commit.stderr or "")
		if "nothing to commit" in out.lower():
			logging.info("No changes to commit")
		else:
			logging.error("git commit failed: %s", out.strip())
			return 4

	return 0


if __name__ == "__main__":
	raise SystemExit(main())

