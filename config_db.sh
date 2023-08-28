#!/bin/bash

echo "***availaing tnsnames.ora***"

sudo chmod 666 /opt/oracle/product/23c/dbhomeFree/network/admin/tnsnames.ora

sudo cp /opt/oracle/product/23c/dbhomeFree/lib/libclntsh.so /opt/oracle/product/23c/dbhomeFree/instantclient/

echo "***DB INSTALLED***" 


# create user for nodered on db

echo " **** create user for nodered **** " 

sudo su - oracle 
# **set the environment variables**
export ORACLE_SID=FREE 
export ORAENV_ASK=NO 
. /opt/oracle/product/23c/dbhomeFree/bin/oraenv
cd $ORACLE_HOME/bin

wget https://raw.githubusercontent.com/badr42/nodered_MQTT_db23c/main/createuser.sql

# lsnrctl status
./sqlplus -S / as sysdba @createuser.sql


