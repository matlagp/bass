import bluetooth
import sys
from flask import Flask, render_template, redirect, request
from .repositories import NodeRepository
from .mqtt import MQTTClient
app = Flask(__name__)

app.config.from_object('config')

MQTTClient().start()

# Define the database object which is imported
# by modules and controllers
node_repository = NodeRepository()

# Sample HTTP error handling
@app.errorhandler(404)
def not_found(error):
    return render_template('404.html'), 404

# from app.mod_auth.controllers import mod_auth as auth_module

# app.register_blueprint(auth_module)

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
            node_repository.update(node)
        except Exception as e:
            print(e)
            return render_template('nodes/edit.html', node=node)
    return render_template('nodes/show.html', node=node)

@app.route('/nodes/<node_id>/edit/')
def nodes_edit(node_id=0):
    return render_template('nodes/edit.html', node=node_repository.find(node_id))

@app.route('/bt/')
def bt():
    nearby_devices = bluetooth.discover_devices(duration=1, lookup_names=True,
                                                flush_cache=False, lookup_class=False)

    new_nodes = [
        (addr, name) for addr, name
        in nearby_devices if name[0:5] == 'node-'
    ]

    return render_template('bt/index.html', new_nodes=new_nodes)

@app.route('/bt/pair/<bt_addr>/')
def bt_pair(bt_addr, bt_port = 1):
    try:
        sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        sock.connect((bt_addr, bt_port))
        print('Connected', file=sys.stderr)
        sock.send('\n'.join(['1foo', '2bar', '3192.168.11.110', '']))
    except Exception as e:
        print(e)
    finally:
        sock.close()
        return redirect('/bt/')

node_repository.create_database()
