import tkinter as tk
from tkinter import ttk, messagebox
import paho.mqtt.client as mqtt
import socket
import ssl
import time
import threading
import struct

# -----------------------------------------------------------------------------
# DEFAULT CONFIGURATIONS
# -----------------------------------------------------------------------------

MQTT_DEFAULTS = {
    "broker": "5aeaff002e7c423299c2d92361292d54.s1.eu.hivemq.cloud",
    "port": 8883,
    "username": "yousef",
    "password": "Yousef123",
    "topic": "com/yousef/esp32/data",
}

UDP_DEFAULTS = {
    "ip": "41.238.164.247",
    "port": 19132,
}

# -----------------------------------------------------------------------------
# CAN ID & PROTOCOL DEFINITIONS
# -----------------------------------------------------------------------------

COMM_CAN_ID_IMU_ANGLE    = 0x071
COMM_CAN_ID_IMU_ACCEL    = 0x072
COMM_CAN_ID_ADC          = 0x073
COMM_CAN_ID_PROX_ENCODER = 0x074
COMM_CAN_ID_GPS_LATLONG  = 0x075
COMM_CAN_ID_TEMP         = 0x076

def create_can_packet(can_id, data_bytes):
    """
    Creates a 20-byte binary packet compatible with the dashboard's unpack logic.
    Format: Padding(4) + ID(4) + DLC(1) + Data(8) + Padding(3)
    """
    if len(data_bytes) < 8:
        data_bytes += b'\x00' * (8 - len(data_bytes))
    elif len(data_bytes) > 8:
        data_bytes = data_bytes[:8]

    header = b'\x00\x00\x00\x00'
    packet_id = struct.pack("<L", can_id)
    dlc = struct.pack("B", 8)
    
    payload = header + packet_id + dlc + data_bytes
    
    if len(payload) < 20:
        payload += b'\x00' * (20 - len(payload))
        
    return payload

def generate_telemetry_packets(norm):
    """
    Calculates values based on normalized slider (0.0 - 1.0) using correct RANGES.
    """
    packets = []

    # --- 1. ADC (0x073) ---
    # Range: 0-1023 (10 bits)
    val_10b = int(norm * 1023) & 0x3FF
    adc_raw = (
        (val_10b) | 
        (val_10b << 10) | 
        (val_10b << 20) | 
        (val_10b << 30) | 
        (val_10b << 40) | 
        (val_10b << 50)
    )
    packets.append(create_can_packet(COMM_CAN_ID_ADC, struct.pack("<Q", adc_raw)))

    # --- 2. PROX_ENCODER (0x074) ---
    # Range: 0-2000 (Updated from source)
    rpm = int(norm * 2000) & 0x7FF
    enc = int(norm * 1023) & 0x3FF
    spd = int(norm * 200) & 0xFF 

    prox_raw = (
        (rpm) |
        (rpm << 11) |
        (rpm << 22) |
        (rpm << 33) |
        (enc << 44) |
        (spd << 54)
    )
    packets.append(create_can_packet(COMM_CAN_ID_PROX_ENCODER, struct.pack("<Q", prox_raw)))

    # --- 3. IMU ANGLE (0x071) ---
    # Range: -180 to 180 (X/Z), -90 to 90 (Y) (Updated from source)
    ang_x = int(-180 + norm * 360)
    ang_y = int(-90 + norm * 180)
    ang_z = int(-180 + norm * 360)
    
    imu_ang_data = struct.pack("<hhh", ang_x, ang_y, ang_z)
    packets.append(create_can_packet(COMM_CAN_ID_IMU_ANGLE, imu_ang_data))

    # --- 4. IMU ACCEL (0x072) ---
    # Range: 0 to 16 (Updated from source)
    accel = int(norm * 16)
    imu_acc_data = struct.pack("<hhh", accel, accel, accel)
    packets.append(create_can_packet(COMM_CAN_ID_IMU_ACCEL, imu_acc_data))

    # --- 5. GPS (0x075) ---
    # Range: Lat 0-180, Lon -180-0 (Updated from source)
    lat = norm * 180.0
    lon = -180.0 + (norm * 180.0)
    gps_data = struct.pack("<ff", lon, lat)
    packets.append(create_can_packet(COMM_CAN_ID_GPS_LATLONG, gps_data))

    # --- 6. TEMP (0x076) ---
    # Range: 0 to 300 (Updated from source)
    temp = int(norm * 300)
    temp_data = struct.pack("<hhhh", temp, temp, temp, temp)
    packets.append(create_can_packet(COMM_CAN_ID_TEMP, temp_data))

    return packets


# -----------------------------------------------------------------------------
# GUI APPLICATION
# -----------------------------------------------------------------------------

class TelemetryApp:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Telemetry Simulator (Fixed)")
        self.root.geometry("550x750")
        self.initialized = False  # PREVENTS CRASH ON STARTUP

        self.mqtt_client = None
        self.mqtt_connected = False
        self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Rate limiting
        self.last_send_time = 0
        self.send_interval = 0.05  # 20 Hz

        # Statistics
        self.packets_sent = 0
        self.last_packet_time = None

        # --- TKinter Variables ---
        self.protocol_var = tk.StringVar(value="MQTT")
        
        # MQTT Vars
        self.mqtt_broker_var = tk.StringVar(value=MQTT_DEFAULTS["broker"])
        self.mqtt_port_var = tk.StringVar(value=str(MQTT_DEFAULTS["port"]))
        self.mqtt_user_var = tk.StringVar(value=MQTT_DEFAULTS["username"])
        self.mqtt_pass_var = tk.StringVar(value=MQTT_DEFAULTS["password"])
        self.mqtt_topic_var = tk.StringVar(value=MQTT_DEFAULTS["topic"])
        self.mqtt_status_var = tk.StringVar(value="Disconnected")

        # UDP Vars
        self.udp_ip_var = tk.StringVar(value=UDP_DEFAULTS["ip"])
        self.udp_port_var = tk.StringVar(value=str(UDP_DEFAULTS["port"]))
        
        self.stats_var = tk.StringVar(value="Frames sent: 0")
        self.mqtt_entries = []
        self.udp_entries = []

        self.create_widgets()
        self.toggle_protocol_ui()
        
        # App is now ready to handle events
        self.initialized = True

    def create_widgets(self):
        main_frame = ttk.Frame(self.root, padding=10)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # --- Protocol Selection ---
        proto_frame = ttk.LabelFrame(main_frame, text="1. Select Protocol", padding=10)
        proto_frame.pack(fill=tk.X, pady=5)
        
        ttk.Radiobutton(proto_frame, text="MQTT (Secure)", variable=self.protocol_var, value="MQTT", command=self.toggle_protocol_ui).pack(side=tk.LEFT, padx=10)
        ttk.Radiobutton(proto_frame, text="UDP", variable=self.protocol_var, value="UDP", command=self.toggle_protocol_ui).pack(side=tk.LEFT, padx=10)

        # --- MQTT Configuration ---
        self.mqtt_frame = ttk.LabelFrame(main_frame, text="2. MQTT Configuration", padding=10)
        self.mqtt_frame.pack(fill=tk.X, pady=5)
        
        self.mqtt_entries.append(self.create_entry(self.mqtt_frame, "Broker:", self.mqtt_broker_var))
        self.mqtt_entries.append(self.create_entry(self.mqtt_frame, "Port:", self.mqtt_port_var))
        self.mqtt_entries.append(self.create_entry(self.mqtt_frame, "Username:", self.mqtt_user_var))
        self.mqtt_entries.append(self.create_entry(self.mqtt_frame, "Password:", self.mqtt_pass_var, show="*"))
        self.mqtt_entries.append(self.create_entry(self.mqtt_frame, "Topic:", self.mqtt_topic_var))
        
        self.mqtt_connect_button = ttk.Button(self.mqtt_frame, text="Connect", command=self.toggle_mqtt_connection)
        self.mqtt_connect_button.pack(pady=5)
        self.mqtt_entries.append(self.mqtt_connect_button)
        
        ttk.Label(self.mqtt_frame, textvariable=self.mqtt_status_var, font=("Arial", 10, "italic")).pack(pady=5)

        # --- UDP Configuration ---
        self.udp_frame = ttk.LabelFrame(main_frame, text="2. UDP Configuration", padding=10)
        self.udp_frame.pack(fill=tk.X, pady=5)

        self.udp_entries.append(self.create_entry(self.udp_frame, "Server IP:", self.udp_ip_var))
        self.udp_entries.append(self.create_entry(self.udp_frame, "Server Port:", self.udp_port_var))

        # --- Simulator Control ---
        sim_frame = ttk.LabelFrame(main_frame, text="3. Simulator Control (ADC Value: 0-4095)", padding=10)
        sim_frame.pack(fill=tk.X, pady=10)

        self.slider_value_label = ttk.Label(sim_frame, text="ADC Value: 0", font=("Arial", 10, "bold"))
        self.slider_value_label.pack(pady=5)

        slider_frame = ttk.Frame(sim_frame)
        slider_frame.pack(fill=tk.X, padx=10, pady=5)

        ttk.Label(slider_frame, text="0").pack(side=tk.LEFT)
        self.slider = ttk.Scale(slider_frame, from_=0, to=4095, orient=tk.HORIZONTAL, command=self.on_slider_move)
        self.slider.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=10)
        ttk.Label(slider_frame, text="4095").pack(side=tk.RIGHT)

        # --- Statistics ---
        stats_frame = ttk.LabelFrame(main_frame, text="Statistics", padding=10)
        stats_frame.pack(fill=tk.X, pady=5)
        ttk.Label(stats_frame, textvariable=self.stats_var, font=("Arial", 9)).pack()

        # --- Data Display ---
        data_frame = ttk.LabelFrame(main_frame, text="Sent Data Hex (Last Batch)", padding=10)
        data_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.data_text = tk.Text(data_frame, height=8, wrap=tk.WORD, font=("Courier", 9))
        self.data_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        self.data_text.insert("1.0", "Move slider to send data...")
        self.data_text.config(state=tk.DISABLED)

        self.slider.set(0)

    def create_entry(self, parent, label_text, var, show=None):
        frame = ttk.Frame(parent)
        frame.pack(fill=tk.X, pady=3)
        ttk.Label(frame, text=label_text, width=12).pack(side=tk.LEFT)
        entry = ttk.Entry(frame, textvariable=var, show=show)
        entry.pack(fill=tk.X, expand=True, side=tk.LEFT)
        return entry

    def toggle_protocol_ui(self):
        if self.protocol_var.get() == "MQTT":
            for widget in self.mqtt_entries: widget.config(state='normal')
            for widget in self.udp_entries: widget.config(state='disabled')
        else:
            for widget in self.mqtt_entries: widget.config(state='disabled')
            for widget in self.udp_entries: widget.config(state='normal')
            if self.mqtt_connected: self.disconnect_mqtt()

        self.packets_sent = 0
        self.update_stats()

    def on_slider_move(self, slider_val_str):
        if not self.initialized: return
        
        current_time = time.time()
        if current_time - self.last_send_time < self.send_interval: return
        self.last_send_time = current_time

        try:
            slider_val = float(slider_val_str)
            self.slider_value_label.config(text=f"ADC Value: {int(slider_val)}")
            
            norm = slider_val / 4095.0
            packets = generate_telemetry_packets(norm)
            
            self.data_text.config(state=tk.NORMAL)
            self.data_text.delete("1.0", tk.END)
            
            display_str = ""
            success_count = 0
            
            for pkt in packets:
                can_id = struct.unpack_from("<L", pkt, 4)[0]
                display_str += f"ID 0x{can_id:03X}: {pkt.hex().upper()}\n"

                sent = False
                if self.protocol_var.get() == "MQTT":
                    sent = self.send_mqtt(pkt)
                else:
                    sent = self.send_udp(pkt)
                if sent: success_count += 1

            self.data_text.insert("1.0", display_str)
            self.data_text.config(state=tk.DISABLED)

            if success_count > 0:
                self.packets_sent += success_count
                self.last_packet_time = current_time
                self.update_stats()
        except Exception as e:
            pass

    def update_stats(self):
        if self.last_packet_time:
            elapsed = time.time() - self.last_packet_time
            self.stats_var.set(f"Frames sent: {self.packets_sent} | Last: {elapsed:.1f}s ago")
        else:
            self.stats_var.set(f"Frames sent: {self.packets_sent}")

    def send_mqtt(self, message_bytes):
        if not self.mqtt_connected:
            self.mqtt_status_var.set("Disconnected - Connect first")
            return False
        try:
            self.mqtt_client.publish(self.mqtt_topic_var.get(), message_bytes, qos=0)
            return True
        except Exception:
            self.mqtt_connected = False
            return False

    def send_udp(self, message_bytes):
        try:
            self.udp_socket.sendto(message_bytes, (self.udp_ip_var.get(), int(self.udp_port_var.get())))
            return True
        except Exception:
            return False

    # --- MQTT Connection ---
    def toggle_mqtt_connection(self):
        if self.mqtt_connected: self.disconnect_mqtt()
        else: self.connect_mqtt()

    def connect_mqtt(self):
        try:
            broker = self.mqtt_broker_var.get()
            port = int(self.mqtt_port_var.get())
            self.mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
            self.mqtt_client.on_connect = self.on_mqtt_connect
            self.mqtt_client.on_disconnect = self.on_mqtt_disconnect
            self.mqtt_client.username_pw_set(self.mqtt_user_var.get(), self.mqtt_pass_var.get())
            self.mqtt_client.tls_set(cert_reqs=ssl.CERT_REQUIRED, tls_version=ssl.PROTOCOL_TLS)
            
            self.mqtt_status_var.set(f"Connecting to {broker}...")
            self.mqtt_connect_button.config(state="disabled")
            
            threading.Thread(target=self._do_mqtt_connect, args=(broker, port), daemon=True).start()
        except Exception as e:
            self.mqtt_status_var.set(f"Error: {e}")
            self.mqtt_connect_button.config(state="normal")

    def _do_mqtt_connect(self, broker, port):
        try:
            self.mqtt_client.connect(broker, port, 60)
            self.mqtt_client.loop_start()
        except Exception as e:
            self.root.after(0, lambda: self.mqtt_status_var.set(f"Conn Fail: {e}"))
            self.root.after(0, lambda: self.mqtt_connect_button.config(state="normal"))

    def disconnect_mqtt(self):
        if self.mqtt_client:
            self.mqtt_client.loop_stop()
            self.mqtt_client.disconnect()
            self.mqtt_connected = False
            self.mqtt_status_var.set("Disconnected")
            self.mqtt_connect_button.config(text="Connect", state="normal")

    def on_mqtt_connect(self, client, userdata, flags, rc, properties=None):
        if rc == 0:
            self.mqtt_connected = True
            self.root.after(0, lambda: self.update_mqtt_ui("Connected", "Disconnect", "normal"))
        else:
            self.mqtt_connected = False
            self.root.after(0, lambda: self.update_mqtt_ui(f"Failed {rc}", "Connect", "normal"))

    def on_mqtt_disconnect(self, client, userdata, rc, properties=None):
        self.mqtt_connected = False
        self.root.after(0, lambda: self.update_mqtt_ui("Disconnected", "Connect", "normal"))

    def update_mqtt_ui(self, status, btn_text, btn_state):
        self.mqtt_status_var.set(status)
        self.mqtt_connect_button.config(text=btn_text, state=btn_state)

    def on_closing(self):
        """Ensures clean shutdown of MQTT thread and sockets."""
        if self.mqtt_connected and self.mqtt_client:
            try:
                self.mqtt_client.loop_stop()
                self.mqtt_client.disconnect()
            except:
                pass
        try:
            self.udp_socket.close()
        except:
            pass
        self.root.quit() # Stops the main loop
        self.root.destroy() # Destroys the window

if __name__ == "__main__":
    root = tk.Tk()
    app = TelemetryApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()
