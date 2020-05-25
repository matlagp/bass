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

@app.route('/nodes')
def nodes_index():
    return render_template('nodes/index.html', nodes=node_repository.all())
    # return '<br>'.join([str(node) for node in node_repository.all()])

@app.route('/nodes/<node_id>')
def nodes_show(node_id=0):
    return str(node_repository.find(node_id))

node_repository.create_database()
