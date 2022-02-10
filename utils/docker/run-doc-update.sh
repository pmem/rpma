#!/usr/bin/env bash
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2018-2022, Intel Corporation
#

set -e

WORKDIR=$(pwd)
USER_NAME="pmem"
BOT_NAME="pmem-bot"
VERSION="master"
SOURCE_BRANCH=${CI_BRANCH}

function set_up_repo() {
	local ORIGIN=$1
	local UPSTREAM=$2
	local REPO_NAME=$3
	local BRANCH=$4

	cd ${WORKDIR}
	git clone ${ORIGIN}
	cd ${REPO_NAME}
	git remote add upstream ${UPSTREAM}

	git config --local user.name ${BOT_NAME}
	git config --local user.email "${BOT_NAME}@intel.com"

	git remote update
	git checkout -B ${BRANCH} upstream/${BRANCH}
}

function commit_and_push_changes() {
	local ORIGIN=$1
	local BRANCH_LOCAL=$2
	local BRANCH_REMOTE=$3
	local TITLE=$4

	# Add, commit and push changes.
	# The 'git commit' command may fail if there is nothing to commit.
	# In such case we want to force push anyway (because there
	# might be an open pull request with changes which were reverted).
	git add -A
	git commit -m "${TITLE}" && true
	git push -f ${ORIGIN} ${BRANCH_LOCAL}

	# Makes pull request.
	# When there is already an open PR or there are no changes
	# an error is thrown, which we ignore.
	GITHUB_TOKEN=${DOC_UPDATE_GITHUB_TOKEN} hub pull-request -f \
		-b ${USER_NAME}:${BRANCH_REMOTE} \
		-h ${BOT_NAME}:${BRANCH_LOCAL} \
		-m "${TITLE}" && true
}

#
# update man pages in the "rpma" repo
#
ORIGIN_RPMA="https://${DOC_UPDATE_GITHUB_TOKEN}@github.com/${BOT_NAME}/rpma"
UPSTREAM_RPMA="https://github.com/${USER_NAME}/rpma"
TARGET_BRANCH="man-pages"

# set up the rpma repo
set_up_repo ${ORIGIN_RPMA} ${UPSTREAM_RPMA} rpma ${SOURCE_BRANCH}

# build docs
mkdir ${WORKDIR}/rpma/build
cd ${WORKDIR}/rpma/build
cmake -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF ..
make -j$(nproc) doc
# copy Markdown files outside the repo
cp -R doc/md ${WORKDIR}

# checkout the 'man-pages' branch
cd ${WORKDIR}/rpma
BRANCH_PR="doc-automatic-update-of-man-pages"
git checkout -B $BRANCH_PR upstream/${TARGET_BRANCH}
git clean -dfx

# clean old content, since some files might have been deleted
DOCS_DIR=${WORKDIR}/rpma/manpages/${VERSION}/
rm -r ${DOCS_DIR}
mkdir ${DOCS_DIR}
# copy new man pages
cp -r ${WORKDIR}/md/*.md ${DOCS_DIR}

# add, commit and push changes to the rpma repo
commit_and_push_changes ${ORIGIN_RPMA} ${BRANCH_PR} ${TARGET_BRANCH} "doc: automatic update of man pages"

exit 0
