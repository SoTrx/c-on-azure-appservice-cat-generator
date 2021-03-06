#!/usr/bin/env ash
# Turn on job management
set -m

# Read all env variable from the container. Used in SSH sessions
eval $(printenv | sed -n "s/^\([^=]\+\)=\(.*\)$/export \1=\2/p" | sed 's/"/\\\"/g' | sed '/=/s//="/' | sed 's/$/"/' >> /etc/profile)

# Start the sshd server
rc-update add sshd
/usr/sbin/sshd

# Start the nginx reverse proxy
nginx&

# Create app service log file
mkdir -p /home/LogFiles
touch /home/LogFiles/dotnet_$WEBSITE_ROLE_INSTANCE_ID_out.log
echo "$(date) Container started" >> /home/LogFiles/dotnet_$WEBSITE_ROLE_INSTANCE_ID_out.log

# If there is any command line argument specified, run it
[ $# -ne 0 ] && su-exec appuser "$@"

# Start the app !
echo "Starting default app..."
cd /app && su-exec appuser /app/cat-generator