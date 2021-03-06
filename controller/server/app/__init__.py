import bluetooth
import copy
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


@app.route('/')
@app.errorhandler(404)
def redirect_to_nodes(*args):
    return redirect('/nodes/')

@app.route('/nodes/')
def nodes_index():
    return render_template('nodes/index.html', nodes=node_repository.all())


@app.route('/nodes/<node_id>/', methods=['GET'])
def nodes_show(node_id=0):
    return redirect('/nodes/')


@app.route('/nodes/<node_id>/', methods=['POST'])
def nodes_update(node_id=0):
    errors = {}
    unsaved = request.values
    new_volume, new_bass, new_mid, new_treble = False, False, False, False

    old_node = node_repository.find(node_id)
    node = copy.deepcopy(old_node)

    def _check_range(value, min_value, max_value, comment):
        if value < min_value or value > max_value:
            errors[comment.lower()] = f"should be between {min_value} and {max_value}"
            return False
        return True

    if (request.values['name']):
        node.name = request.values['name']

    if (request.values['volume']):
        try:
            volume = int(request.values['volume'])
            if volume != node.volume and _check_range(volume, 0, 100, "Volume"):
                new_volume = True
                node.volume = volume
        except ValueError:
            errors['volume'] = "should be an integer"

    if (request.values['bass']):
        try:
            bass = float(request.values['bass'])
            if bass != node.bass and _check_range(bass, -24, 12, "Bass"):
                new_bass = True
                node.bass = bass
        except ValueError:
            errors['bass'] = "should be a number"

    if (request.values['mid']):
        try:
            mid = float(request.values['mid'])
            if mid != node.mid and _check_range(mid, -24, 12, "Mid"):
                new_mid = True
                node.mid = mid
        except ValueError:
            errors['mid'] = "should be a number"

    if (request.values['treble']):
        try:
            treble = float(request.values['treble'])
            if treble != node.treble and _check_range(treble, -24, 12, "Treble"):
                new_treble = True
                node.treble = treble
        except ValueError:
            errors['treble'] = "should be a number"


    if not errors:
        if new_volume:
            mqtt_client.publish_node_setting(node.hex_id, 'volume', node.volume)
        if new_bass:
            mqtt_client.publish_node_setting(node.hex_id, 'bass', node.bass)
        if new_mid:
            mqtt_client.publish_node_setting(node.hex_id, 'mid', node.mid)
        if new_treble:
            mqtt_client.publish_node_setting(node.hex_id, 'treble', node.treble)
        node_repository.update(node)
        nodes = node_repository.all()
        return render_template('nodes/index.html', nodes=nodes)
    else:
        nodes = node_repository.all()
        return render_template('nodes/index.html', nodes=nodes, edited_node=old_node, errors=errors, unsaved=unsaved)


@app.route('/nodes/<node_id>/edit/')
def nodes_edit(node_id=0):
    try:
        nodes = node_repository.all()
        edited_node = node_repository.find(node_id)
        return render_template('nodes/index.html', nodes=nodes, edited_node=edited_node, errors={}, unsaved={})
    except:
        return redirect('/nodes/')


@app.route('/bt/')
def bt():
    nearby_devices = bluetooth.discover_devices(
        duration=1,
        lookup_names=True,
        flush_cache=False,
        lookup_class=False
    )

    new_nodes = [
        (addr, name[5:]) for addr, name
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


@app.route('/player/')
def player():
    player_url = os.getenv('PLAYER_URL')
    return render_template('player/index.html', player_url=player_url)

node_repository.create_database()
