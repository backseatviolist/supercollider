#!/usr/bin/env bash
# Creates assets for SuperCollider in the form of
# 'SuperCollider-x.x.x-Source.tar.bz2' and moves the file to the repository's
# package folder.
# Requires a writable /tmp folder.

set -euo pipefail

get_absolute_path() {
  echo "$(cd "$(dirname "$0")" && cd .. && pwd -P)"
}

remove_source_dir() {
  echo "Removing potential previous sources."
  rm -rf "${source_dir}/SuperCollider"*
}

checkout_project() {
  remove_source_dir
  echo "Copying project..."
  cd "$git_repo_dir"
  mkdir "${source_dir}/${package_name}"
  git checkout-index -a -f --prefix "${source_dir}/${package_name}/"
}

clean_sources() {
  cd "${source_dir}/${package_name}"
  echo "Removing unneeded files and folders..."
  if [[ -n ${exclusions_file} ]]; then
    for file in $(cat ${exclusions_file}); do
      rm -rfv "${file}"
    done
  fi
}

rename_sources() {
  cd "${source_dir}"
  mv -v "${package_name}" "${package_name}-Source"
}

compress_sources() {
  cd "${source_dir}"
  tar cfz "${package_name}-${version}-Source${suffix}.tar.bz2" \
    "${package_name}-Source"
}

move_sources() {
  cd "${source_dir}"
  mv -v "${package_name}-${version}-Source${suffix}.tar.bz2" "${output_dir}"
}

cleanup_source_dir() {
  cd "${source_dir}"
  rm -rf "${package_name}-${version}"
}

print_help() {
  echo "Options:"
  echo "-v <version>: Version string such as 3.9.0."
  echo "-l: Produce Linux package."
  exit 1
}

git_repo_dir=$(get_absolute_path $0)
output_dir=${git_repo_dir}/package
package_name="SuperCollider"
source_dir="/tmp"
version=`date "+%Y-%m-%d"`
suffix=""
exclusions_file=""

if [ ${#@} -gt 0 ]; then
  while getopts 'hv:s:l' flag; do
    case "${flag}" in
      h) print_help
          ;;
      v) version=$OPTARG
          ;;
      l) suffix=-linux
         exclusions_file="$git_repo_dir/package/LinuxExclusions.txt"
          ;;
      *)
        echo "Error! Try '${0} -h'."
        exit 1
        ;;
    esac
  done
else
  print_help
fi

checkout_project
clean_sources
rename_sources
compress_sources
move_sources
cleanup_source_dir

exit 0

# vim:set ts=2 sw=2 et:
