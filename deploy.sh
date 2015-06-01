#!/bin/bash

declare -r EXCLUDES=$(dirname $BASH_SOURCE)/exclude.txt
declare -r REPO_ROOT=$(dirname $BASH_SOURCE)

if [ "$1" = "dev" ]; then
	declare -r TARGET_DIR=/var/www/html/chantico
elif [ "$1" = "prod" ]; then
	declare -r TARGET_DIR=login@mywadapi.com:api
else
	echo "Please specify one of [dev/prod] as deploy target"
	exit
fi

if [ "$2" = "go" ];then
	rsync -rltzuv --itemize-changes --delete -O --exclude-from $EXCLUDES $REPO_ROOT $TARGET_DIR
else
	rsync -rltzuv --itemize-changes --delete -O --dry-run --exclude-from $EXCLUDES $REPO_ROOT $TARGET_DIR
fi

