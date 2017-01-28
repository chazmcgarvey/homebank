#!/bin/sh

: <<'=cut'
=pod

=head1 NAME

import_upstream.sh - Import HomeBank from upstream

=head1 SYNOPSIS

    ./maint/import_upstream.sh BRANCH

    # import the latest release on the 5.1.x branch
    ./maint/import_upstream.sh 5.1.x

=head1 DESCRIPTION

This script downloads the latest HomeBank release on a given branch and adds
that release to the local C<upstream> branch.

=cut

export GIT_AUTHOR_NAME='Maxime Doyen'
export GIT_AUTHOR_EMAIL='homebank@free.fr'

branch_git='upstream'
branch_bzr=$1
shift

if [ -z "$branch_bzr" ]
then
    echo >&2 "$0: Must specify homebank branch (e.g. \"5.1.x\")"
    exit 1
fi

set -e

# download homebank
cleanup='rm -rf "$branch_bzr"'
eval "$cleanup"
trap "$cleanup" EXIT
bzr branch "lp:homebank/$branch_bzr" "$branch_bzr"

# determine version
hb_version=$(perl <"$branch_bzr/src/homebank.h" -ne 's/#define HB_VERSION\s+"(.*)"/$1/ && print')
if [ -z "$hb_version" ]
then
    echo >&2 "$0: Cannot determine homebank version"
    exit 1
fi

cp .gitignore "$branch_bzr/"

# commit new version of homebank
git add "$branch_bzr"
tree_ref=$(git write-tree --prefix="$branch_bzr/")
git reset "$branch_bzr"
parent_ref=$(git rev-parse "$branch_git")
commit_ref=$(git commit-tree -m "import homebank-$hb_version" -p $parent_ref $tree_ref)

# update branch
git branch -f "$branch_git" $commit_ref

