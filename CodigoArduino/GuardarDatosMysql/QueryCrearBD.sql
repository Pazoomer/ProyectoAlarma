CREATE DATABASE alarma;
USE alarma;
# Asegúrate de tener esta tabla creada antes:
CREATE TABLE datos_alarma (
     id INT AUTO_INCREMENT PRIMARY KEY,
     estado VARCHAR(10),
     ruido INT,
     movimiento INT,
     magnetico INT,
     fecha TIMESTAMP DEFAULT CURRENT_TIMESTAMP
 );