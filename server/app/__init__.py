import bluetooth
import sys
from flask import Flask, render_template
from .repositories import NodeRepository
app = Flask(__name__)

app.config.from_object('config')

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
    # return '<br>'.join([str(node) for node in node_repository.all()])

@app.route('/nodes/<node_id>/')
def nodes_show(node_id=0):
    return str(node_repository.find(node_id))

@app.route('/bt/')
def bt():
    nearby_devices = bluetooth.discover_devices(duration=8, lookup_names=True,
                                                flush_cache=False, lookup_class=False)

    ret = "Found {} devices".format(len(nearby_devices))
    for addr, name in nearby_devices:
        try:
            ret += "<br>{} - {}".format(addr, name)
        except UnicodeEncodeError:
            ret += "<br>{} - {}".format(addr, name.encode("utf-8", "replace"))
    return ret

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
        return "done"

node_repository.create_database()
