import threading

from flask import Flask, render_template, request, jsonify, flash, redirect, url_for, send_file, send_from_directory
from werkzeug.utils import secure_filename
import requests
import pywifi
import json
import csv
import os
import socket

CSV_NAME = "dimmer_tuning.csv"
UPLOAD_FOLDER = './uploads'
ALLOWED_EXTENSIONS = {'bin'}

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER


def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


''' Configure Base WiFi Objects '''
wifi = pywifi.PyWiFi()

iface = wifi.interfaces()[0]
iface.scan()

profile = pywifi.Profile()
profile.auth = pywifi.const.AUTH_ALG_OPEN
profile.akm.append(pywifi.const.AKM_TYPE_WPA2PSK)
profile.cipher = pywifi.const.CIPHER_TYPE_CCMP
profile.key = "automation"

''' Create csv file if it doesn't exist '''
field_names = ['bulbModel', 'triacPulsePercentage', 'brightnessCeilingPercentage', 'brightnessFloorPercentage',
               'triacMinPulseMicroSeconds']

csv_exist = os.path.exists(CSV_NAME)
if not csv_exist:
    with open("dimmer_tuning.csv", 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(field_names)


@app.route('/scan')
def scan():
    detected_aps = iface.scan_results()
    infitnite_ssids = {ap.ssid for ap in detected_aps if ap.ssid.startswith("dm_infinite_")}
    iface.scan()
    return json.dumps({"ssids": list(infitnite_ssids)})


@app.route('/connect/<ssid>')
def connect(ssid):
    iface.disconnect()
    iface.remove_all_network_profiles()
    profile.ssid = ssid
    iface.add_network_profile(profile)
    iface.connect(profile)
    return "CONNECTED!"


@app.route('/export', methods=['POST'])
def export():
    raw_data = request.get_data().decode()
    data_dict = json.loads(raw_data)
    with open("dimmer_tuning.csv", 'a', newline='') as f:
        dictwriter = csv.DictWriter(f, fieldnames=field_names)
        dictwriter.writerow(data_dict)
    return "hello export"


def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('192.255.255.255', 1))
        IP = s.getsockname()[0]
    except:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP


@app.route('/upload', methods=['POST'])
def upload_file():
    ''' NOTE: It appears as if the ESP32 can't work out local routes while in AP mode''' 
    if 'file' not in request.files:
        flash('No file part')
        return redirect(request.url)
    file = request.files['file']
    # If the user does not select a file, the browser submits an
    # empty file without a filename.
    if file.filename == '':
        flash('No selected file')
        return redirect(request.url)
    if file and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
        local_ip = get_local_ip()
        # NOTE: Wifi seems to be required for OTA.
        return json.dumps({"ip": local_ip, "filename": filename})


@app.route('/download/<filename>')
def download_file(filename):
    print(filename)
    return send_from_directory(app.config["UPLOAD_FOLDER"], filename)


@app.route('/')
def index():
    print(wifi.interfaces())
    return render_template("index.html")


if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)
