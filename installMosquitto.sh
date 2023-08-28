#!/bin/sh
# This file can be used as an auto installer or you can mannually execute the steps yourself.
# If you use it as an installer, you must run it as root.

# Check if the script is run with sudo
if [ "$EUID" -ne 0 ]
  then echo "Please run as root or with sudo"
  exit
else
  echo "Installing Mosquitto..."
fi

# Install epel-release and enable PowerTools
dnf install -y epel-release
dnf config-manager --set-enabled PowerTools

# Download and install libwebsockets and mosquitto:
curl -LO https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/l/libwebsockets-3.0.1-2.el7.x86_64.rpm
curl -LO https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/m/mosquitto-1.6.10-1.el7.x86_64.rpm
dnf install -y libwebsockets-3.0.1-2.el7.x86_64.rpm
dnf install -y mosquitto-1.6.10-1.el7.x86_64.rpm

# Create the log directory. Without this the mosquitto systemd service will fail.
mkdir /var/log/mosquitto
touch /var/log/mosquitto/mosquitto.log
chown -R mosquitto:mosquitto /var/log/mosquitto

# Remove leftover files
rm -f libwebsockets-3.0.1-2.el7.x86_64.rpm
rm -f mosquitto-1.6.10-1.el7.x86_64.rpm

# Exit message

echo "Installation complete!"