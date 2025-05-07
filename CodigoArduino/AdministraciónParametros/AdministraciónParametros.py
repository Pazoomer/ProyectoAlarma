import serial
import time
import threading
import tkinter as tk
from tkinter import ttk, scrolledtext

class AlarmaGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Control de Alarma ESP32")

        self.ser = None
        self.running = False

        self.create_widgets()  # Crear interfaz primero

        # Intentar abrir puerto
        try:
            self.ser = serial.Serial('COM6', 115200, timeout=1)
            time.sleep(2)
            self.running = True
            threading.Thread(target=self.read_serial, daemon=True).start()
            self.write_output("✅ Conexión Serial exitosa en COM6")
        except serial.SerialException as e:
            self.write_output(f"❌ Error al abrir puerto COM6: {e}")

    def create_widgets(self):
        frame = ttk.Frame(self.root)
        frame.pack(pady=5)

        ttk.Button(frame, text="Activar Alarma (on)", command=lambda: self.send_command("on")).grid(row=0, column=0, padx=5)
        ttk.Button(frame, text="Desactivar Alarma (off)", command=lambda: self.send_command("off")).grid(row=0, column=1, padx=5)

        ajustes = [
            ("Magnetico (mag x)", "mag"),
            ("Movimiento (mov x)", "mov"),
            ("Ruido (rui x)", "rui"),
            ("Luz (luz x)", "luz"),
            ("Volumen (vol x)", "vol"),
        ]

        for label, cmd in ajustes:
            ttk.Label(self.root, text=label).pack()
            s = ttk.Scale(self.root, from_=0, to=10, orient="horizontal", command=lambda val, c=cmd: self.send_command(f"{c} {int(float(val))}"))
            s.pack(fill="x", padx=10)

        self.output = scrolledtext.ScrolledText(self.root, width=60, height=15, state="disabled")
        self.output.pack(pady=10)

    def send_command(self, cmd):
        if self.ser and self.ser.is_open:
            try:
                self.ser.write((cmd + "\n").encode())
                self.write_output(f">>> {cmd}")
            except serial.SerialException as e:
                self.write_output(f"❌ Error enviando comando: {e}")
        else:
            self.write_output("⚠ No hay conexión serial activa")

    def read_serial(self):
        while self.running:
            try:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode(errors="ignore").strip()
                    if line:
                        self.write_output(line)
            except serial.SerialException as e:
                self.write_output(f"❌ Error leyendo del puerto: {e}")
                self.running = False

    def write_output(self, text):
        self.output.config(state="normal")
        self.output.insert(tk.END, text + "\n")
        self.output.yview(tk.END)
        self.output.config(state="disabled")

    def on_close(self):
        self.running = False
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except serial.SerialException:
                pass
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = AlarmaGUI(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
