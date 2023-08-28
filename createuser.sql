CREATE USER C##nodered IDENTIFIED BY Password123;
GRANT CREATE SESSION TO C##nodered;
GRANT CREATE TABLE TO C##nodered;
ALTER USER C##NODERED QUOTA UNLIMITED ON USERS;
CREATE TABLE C##nodered.nodered (col1 varchar2(500),col2 varchar2(500));
CREATE TABLE C##nodered.sensor_data (
  id NUMBER GENERATED ALWAYS AS IDENTITY, -- Optional auto-generated ID column
  vibration NUMBER,
  sound NUMBER,
  temp NUMBER,
  humi NUMBER,
  timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- INSERT INTO C##nodered.sensor_data (vibration, sound, temp, humi) VALUES (6.87, 549, 30.53, 38.2);
-- select count(*) from C##nodered.sensor_data
exit;