#!/usr/bin/bash

set -e
[ $UID -ne 0 ] && {
	sed -i '/^[^#]*clear_console/s/^\s*/\0: #/g' .bash_logout
} || { [ -n "$1" ] && [ -z "${1#"${1%%[^0-9]*}"}" ]; } || {
	printf "Run this as yourself and let it ask for privileges\n" >&2
	exit 1
}
[ $EUID -eq 0 ] || {
	sudo "${BASH_SOURCE[0]}" "$UID"
	exit $?
}
command -v gitlab-runner &>/dev/null || {
	# shellcheck disable=SC2144
	[ -f /etc/apt/sources.list.d/*gitlab-runner* ] || {
		chattr -i /etc/apt/sources.list.d
		# can GitLab not just package their things for official repos???
		curl -L https://packages.gitlab.com/install/repositories/runner/gitlab-runner/script.deb.sh | bash || {
			chattr -R +i /etc/apt/sources.list.d
			exit 1
		}
		chattr -R +i /etc/apt/sources.list.d
	}
	# don't want the post-installation script to run
	# shellcheck disable=SC2015
	apt-get download gitlab-runner && \
	dpkg --unpack gitlab-runner_*.deb && \
	rm -f /var/lib/dpkg/info/gitlab-runner.postinst && \
	dpkg --configure gitlab-runner && \
	apt-get install -yf || {
		rm -f gitlab-runner_*.deb
		exit 1
	}
	rm -f gitlab-runner_*.deb
}
command -v git &>/dev/null || apt-get install git git-lfs
command -v git-lfs &>/dev/null || apt-get install git-lfs
git lfs install
command -v clang-format || apt-get install clang-format
command -v shellcheck || apt-get install shellcheck
command -v cppcheck || apt-get install cppcheck
mkdir -p /builds
chown -R "$1:$(stat -c '%g' "$(eval printf "%s" "~$(id -un "$1")")")" /builds
printf "GitLab URL:  %s\n" "https://gitgis.ecorp.cat.com"
printf "Executor:    %s\n" "docker"
sudo -u "#$1" gitlab-runner register
cat <<- __EOF__ > /etc/systemd/system/gitlab-runner.service
	[Unit]
	Description=GitLab Runner
	Wants=network.target
	After=network.target

	[Service]
	User=$1
	Group=$1
	ExecStart=gitlab-runner run
	Restart=always

	[Install]
	WantedBy=multi-user.target
__EOF__
systemctl daemon-reload
systemctl enable --now gitlab-runner
printf "\nSet up docker image with \$HOME/repos mounted\n"
