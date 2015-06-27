#!/bin/bash

declare -r EXCLUDES=$(dirname $BASH_SOURCE)/exclude.txt
declare -r REPO_ROOT=$(dirname $BASH_SOURCE)

if [ "$1" = "sandbox" ]; then
	declare -r TARGET_DIR=loginsandbox@mywadapi.com:api
elif [ "$1" = "prod" ]; then
	declare -r TARGET_DIR=login@mywadapi.com:api
else
	echo "Please specify one of [dev/prod] as deploy target"
	exit
fi

if [ "$2" = "go" ];then
	#Copy Relevant Settings File
	cp settings.$1.xml settings.xml
	rsync -rltzuv --itemize-changes --delete -O --exclude-from $EXCLUDES $REPO_ROOT $TARGET_DIR
else
	rsync -rltzuv --itemize-changes --delete -O --dry-run --exclude-from $EXCLUDES $REPO_ROOT $TARGET_DIR
fi

