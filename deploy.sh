#!/bin/bash

declare -r EXCLUDES=$(dirname $BASH_SOURCE)/exclude.txt
declare -r REPO_ROOT=$(dirname $BASH_SOURCE)

if [ "$1" = "dev" ]; then
	cp wadadli/conf/settings.dev.xml wadadli/conf/settings.xml
	declare -r TARGET_DIR=/var/www/html/chantico
elif [ "$1" = "prod" ]; then
	cp wadadli/conf/settings.prod.xml wadadli/conf/settings.xml
	declare -r TARGET_DIR=datashanty@datashanty.com:public_html/gbsv/chantico
else
	echo "Please specify one of [dev/prod] as deploy target"
	exit
fi

if [ "$2" = "go" ];then
	rsync -rltzuv --itemize-changes --delete -O --exclude-from $EXCLUDES $REPO_ROOT $TARGET_DIR
else
	rsync -rltzuv --itemize-changes --delete -O --dry-run --exclude-from $EXCLUDES $REPO_ROOT $TARGET_DIR
fi

