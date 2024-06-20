#!/usr/bin/env bash
#
# SPDX-License-Identifier: GPL-3.0
#

OUTPUT_PATH="$1"
GENIMAGE_ARG="$2"
GENIMAGE_SCRIPT="$3"
BOARD_NAME="$4"

function download_and_set_latest_syterkit() {
	echo "Finding SyterKit latest version" "from GitHub" "info"

	# Find the latest version of SyterKit from GitHub, using JSON API, curl and jq.
	declare api_url="https://api.github.com/repos/YuzukiHD/SyterKit/releases/latest"
	declare latest_version
	latest_version=$(curl -s "${api_url}" | jq -r '.tag_name')
	echo "Latest version of SyterKit is" "${latest_version}"

	# Prepare the cache dir
	declare syterkit_cache_dir="${BINARIES_DIR}/SyterKit"
	mkdir -p "${syterkit_cache_dir}"

	declare syterkit_img_filename="${BOARD_NAME}.tar.gz"
	declare -g -r syterkit_img_path="${syterkit_cache_dir}/${syterkit_img_filename}"
	echo "SyterKit image path" "${syterkit_img_path}"

	declare download_url="https://github.com/YuzukiHD/SyterKit/releases/download/${latest_version}/${syterkit_img_filename}"

	# Download the image (with wget) if it doesn't exist; download to a temporary file first, then move to the final path.
	if [[ ! -f "${syterkit_cache_dir}/${BOARD_NAME}/extlinux_boot/extlinux_boot_bin_card.bin" ]]; then
		echo "Downloading SyterKit image" "${download_url}"
		declare tmp_syterkit_img_path="${syterkit_img_path}.tmp"
		wget -O "${tmp_syterkit_img_path}" "${download_url}"
		mv -v "${tmp_syterkit_img_path}" "${syterkit_img_path}"
		echo " Decompressing SyterKit image to" "${syterkit_img_path}/${BOARD_NAME}"
		mkdir -p ${syterkit_cache_dir}/${BOARD_NAME}
		tar -zxf ${syterkit_img_path} -C ${syterkit_cache_dir}/${BOARD_NAME}
	else
		echo "SyterKit image already downloaded, using it" "${syterkit_img_path}"
	fi

	echo "SyterKit copy bootloader and files"
	cp -raf ${syterkit_cache_dir}/${BOARD_NAME}/extlinux_boot/extlinux_boot_bin_card.bin ${BINARIES_DIR}
	echo "${BR2_EXTERNAL_AVAOTASBC_PATH}/board/${BOARD_NAME}/bin/"
	cp -raf ${BR2_EXTERNAL_AVAOTASBC_PATH}/board/${BOARD_NAME}/bin/* ${BINARIES_DIR}
}

download_and_set_latest_syterkit
