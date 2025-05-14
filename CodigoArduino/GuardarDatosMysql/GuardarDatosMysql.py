# Para correrlo poner esto en terminal: C:\Python313\python.exe GuardarDatosMysql.py 
# cd C:\Github\SistemasEmpotrados\ProyectoAlarma\ProyectoAlarma\CodigoArduino\GuardarDatosMysql
import serial
import time
import pymysql

# Función para intentar conectar al puerto serial
def connect_serial(port, baud_rate=115200, timeout=1):
    while True:
        try:
            ser = serial.Serial(port, baud_rate, timeout=timeout)
            print(f"Conexión al puerto {port} establecida.")
            return ser
        except serial.SerialException:
            print(f"Error al conectar al puerto {port}. Intentando nuevamente...")
            time.sleep(2)  # Espera 2 segundos antes de intentar nuevamente

# Conectar a MySQL
def connect_mysql():
    while True:
        try:
            db = pymysql.connect(
                host="localhost",
                user="root",
                password="password",
                database="alarma"
            )
            cursor = db.cursor()
            print("Conexión a MySQL establecida.")
            return db, cursor
        except pymysql.MySQLError as e:
            print(f"Error al conectar a MySQL: {e}. Intentando nuevamente...")
            time.sleep(2)  # Espera 2 segundos antes de intentar nuevamente

# Conectar al puerto serial
ser = connect_serial('COM6')

# Conectar a la base de datos MySQL
db, cursor = connect_mysql()

try:
    while True:
        if ser.in_waiting:
            line = ser.readline().decode(errors='ignore').strip()
            print("Recibido:", line)

            if line.startswith("MYSQL ESTADO:"):
                parts = line.split()
                estado = parts[2]
                ruido = parts[4]
                movimiento = parts[6]
                magnetico = parts[8]

                sql = "INSERT INTO datos_alarma (estado, ruido, movimiento, magnetico) VALUES (%s, %s, %s, %s)"
                cursor.execute(sql, (estado, ruido, movimiento, magnetico))
                db.commit()
                print("Datos guardados en MySQL")

        # Si se pierde la conexión al puerto serial
        if not ser.is_open:
            print("Conexión al puerto serial perdida. Intentando reconectar...")
            ser = connect_serial('COM6')

except KeyboardInterrupt:
    print("Saliendo...")

finally:
    if ser.is_open:
        ser.close()
    db.close()