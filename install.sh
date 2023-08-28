#!/bin/bash


source ~/.bashrc 

#Install nodered
echo "**Installing NODERED**"
sudo yum remove nodejs -y
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.38.0/install.sh | bash
source ~/.bashrc
nvm install node

sudo dnf install -y npm
sudo npm install -g --unsafe-perm node-red


echo "**adding firewall rules**"

sudo systemctl restart dbus
sudo systemctl start firewalld

sleep 15 
sudo firewall-cmd --zone=public --add-port=1880/tcp --permanent
sudo firewall-cmd --zone=public --add-port=1883/tcp --permanent

sudo firewall-cmd --reload



echo "**Adding mods to NodeRed**"
sudo npm install -g -y node-red-contrib-oracledb-mod



echo "*** installing instant client**"

sudo npm install -g -y oracledb





echo "*** create the service ***"

sudo npm install -g pm2

sleep 15



#start nodered

npx pm2 start /usr/local/bin/node-red -- -v --uid opc --gid opc
sleep 5
npx pm2 startup systemd



echo "*** installing nodered flow **"
wget https://raw.githubusercontent.com/badr42/nodered_MQTT_db23c/main/import_nodered.sh
bash import_nodered.sh




echo "*** installing MQTT ***" 
wget https://raw.githubusercontent.com/badr42/nodered_MQTT_db23c/main/installMosquitto.sh
sudo chmod 777 installMosquitto.sh
sudo bash installMosquitto.sh

echo "*** Starting the Service ***" 
sudo systemctl start mosquitto
sudo systemctl enable mosquitto


echo "*************** INSTALLATION COMPLETE ***************"

