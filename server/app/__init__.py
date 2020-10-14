import bluetooth
import sys
import os
import socket
from flask import Flask, render_template, redirect, request
from .repositories import NodeRepository
from .mqtt import MQTTClient
app = Flask(__name__)

app.config.from_object('config')

mqtt_client = MQTTClient()
mqtt_client.start()

node_repository = NodeRepository()


@app.errorhandler(404)
def not_found(error):
    return render_template('404.html'), 404


@app.route('/nodes/')
def nodes_index():
    return render_template('nodes/index.html', nodes=node_repository.all())


@app.route('/nodes/<node_id>/', methods=['GET'])
def nodes_show(node_id=0):
    return render_template('nodes/show.html', node=node_repository.find(node_id))


@app.route('/nodes/<node_id>/', methods=['POST'])
def nodes_update(node_id=0):
    node = node_repository.find(node_id)
    if (request.values['volume']):
        try:
            volume = int(request.values['volume'])
            if volume < 0 or volume > 100:
                raise ValueError("Volume not between 0 and 100")
            node.volume = volume
            mqtt_client.client.publish("/nodes/{0:08X}/settings/volume".format(node.id), node.volume, retain=True)
        except Exception as e:
            print(e)
            return render_template('nodes/edit.html', node=node)
    return render_template('nodes/show.html', node=node)


@app.route('/nodes/<node_id>/edit/')
def nodes_edit(node_id=0):
    return render_template('nodes/edit.html', node=node_repository.find(node_id))


@app.route('/bt/')
def bt():
    nearby_devices = bluetooth.discover_devices(
        duration=1,
        lookup_names=True,
        flush_cache=False,
        lookup_class=False
    )

    new_nodes = [
        (addr, name) for addr, name
        in nearby_devices if name[0:5] == 'node-'
    ]

    return render_template('bt/index.html', new_nodes=new_nodes)


@app.route('/bt/pair/<bt_addr>/')
def bt_pair(bt_addr, bt_port=1):
    def get_ip():
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            # doesn't even have to be reachab
            s.connect(('10.255.255.255', 1))
            IP = s.getsockname()[0]
        except Exception:
            IP = '127.0.0.1'
        finally:
            s.close()
        return IP
    try:
        sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        sock.connect((bt_addr, bt_port))
        print('Connected', file=sys.stderr)
        sock.send('\n'.join([
            f"1{os.getenv('WIFI_SSID')}",
            f"2{os.getenv('WIFI_PASSWORD')}",
            f"3{get_ip()}",
            ''
        ]))
    except Exception as e:
        print(e)
    finally:
        sock.close()
        return redirect('/bt/')


node_repository.create_database()
