import network
import usocket as socket
  

def wifi_connect(ssid, password):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('connecting to network...')
        print('SSID = ' + ssid + ' password = ' + password)
        wlan.connect(ssid, password)
        while not wlan.isconnected():
            pass
    print('network config:', wlan.ifconfig())
    

def start_apn():
    ap = network.WLAN(network.AP_IF)
    ap.config(essid='ESP32-APN')
    ap.config(max_clients=10)
    ap.active(True)


def parse_data(request):
    print(request)
    ssid_data, password_data = request.split(',', 1)
    ssid = ssid_data.split('=')[1]
    password = password_data.split('=')[1]
    print('Parsed data SSID = ' + ssid + ' password = ' + password)
    return ssid, password[:-1];
    

def start_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', 80))
    s.listen(5)
    conn, addr = s.accept()
    print('New connection from %s' % str(addr))
    request = conn.recv(1024).rstrip()
    print('Content = %s' % str(request))
    ssid, password = parse_data(str(request))
    conn.send('OK')
    conn.close()
    return ssid, password;
    
    
start_apn()
print('APN started')
ssid, password = start_server()
print('Server stopped')
wifi_connect(ssid, password)